#include "src/hook.h"
#include "src/iomanager.h"
#include "src/log.h"
#include "src/fdmanager.h"

#include <string.h>
#include <fcntl.h>

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

IOManager * IOManager::GetCurIOM(){
	return dynamic_cast<IOManager *>(IOManager::GetCurScheduler());
}

IOManager::IOManager(uint32_t threadcount , bool usercaller,
		  const std::string & name)
		  :Scheduler(threadcount,usercaller,name){	
	m_epfd = epoll_create(2000);
	if(m_epfd == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"epoll_create no success errno :"<<errno
								   << " strerr : "<< strerror(errno);
		throw std::logic_error("epoll_create no success");
	}
	
	int ret = pipe(m_tickles);
	int fg = fcntl(m_tickles[0],F_GETFL,0);
	fg |= O_NONBLOCK;
	fcntl(m_tickles[0],F_SETFL,fg);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"pipe no success errno : "<< errno 
								   << " strerr : " << strerror(errno);
		throw std::logic_error("pipe no success");
	}

	epoll_event event ; 
	memset(&event, 0 , sizeof(event));
	event.events = EPOLLIN | EPOLLET ; 
	event.data.fd = m_tickles[0];

	ret = epoll_ctl(m_epfd,EPOLL_CTL_ADD,m_tickles[0],&event);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"epoll_ctl no success errno : "<<errno 
								   <<" strerr : "<< strerror(errno);
		throw std::logic_error("epoll_ctl no success");
	}

	resizeFdsSize(64);
	start();
}

void IOManager::EventHandle::triggered(Event ev){
	MutexType::Lock lock(mutex);
	if(ev & READ){
		if(read.cb){
			read.iom->schedule(read.cb);
		}else{
			read.iom->schedule(read.fiber);	
		}
		read.reset();
	}
	if(ev & WRITE){
		if(write.cb){
			write.iom->schedule(write.cb);
		}else{
			write.iom->schedule(write.fiber);
		}
		write.reset();
	}
}

void IOManager::resizeFdsSize(int size){
	MutexType::WRLock lock(m_mutex);
	m_fds.resize(size);
	for(uint32_t i = 0 ; i < m_fds.size(); ++i){
		if(!m_fds[i]){
			m_fds[i] = new EventHandle(i);
		}
	}
}

IOManager::~IOManager(){
//	TADPOLE_LOG_DEBUG(g_logger)<< "~IOManager";
	std::vector<Timer::ptr> vec;
	clearTimerToVec(vec);
	for(auto &it : vec){
		schedule(it->getCb());
	}
	stop();
//	TADPOLE_LOG_ERROR(g_logger)<<"stop success";
	close(m_epfd);
	close(m_tickles[0]);
	close(m_tickles[1]);
	for(uint32_t i = 0 ; i < m_fds.size(); ++i){
		if(m_fds[i]){
			delete m_fds[i];
		}
	}
}

bool IOManager::addEvent(int fd, Event event,std::function<void()> cb){
	MutexType::RDLock lock(m_mutex);
	if(fd >= (int)m_fds.size()){
		lock.unlock();
		resizeFdsSize(fd*1.5);
		lock.lock();
	}
	EventHandle * fd_event = m_fds[fd];
	lock.unlock();
	
	EventHandle::MutexType::Lock lock2(fd_event->mutex);
	int op = (fd_event->event & NONE) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
	fd_event->event = (Event)(fd_event->event | event); 
	int events = EPOLLET | fd_event->event;
	lock2.unlock();
	
	epoll_event env ; 
	memset(&env , 0 , sizeof(env));
	env.events = events;
	env.data.ptr = fd_event;
	int ret = epoll_ctl(m_epfd,op,fd,&env);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"epoll_ctl no success errno : "<<errno 
								   <<" strerr : "<< strerror(errno);
		 return false;
	}
	++m_eventCount;

	EventHandle::MutexType::Lock lock3(fd_event->mutex);
	if(event & READ){
		if(cb){
			fd_event->read.cb = cb;
		}else{
			fd_event->read.fiber = Fiber::GetCurFiber();
		}
		fd_event->read.iom = this;
	}
	
	if(event & WRITE){
		if(cb){
			fd_event->write.cb = cb;
		}else{
			fd_event->write.fiber = Fiber::GetCurFiber();
		}
		fd_event->write.iom = this;
	}	
	return true;
}

bool IOManager::delEvent(int fd, Event event){
	if(fd >= (int)m_fds.size()){
		return false;
	}
	MutexType::RDLock lock(m_mutex);
	EventHandle * fd_event = m_fds[fd];
	if(!(fd_event->event & event)){
		return false; 
	}
	lock.unlock();
	
	
	EventHandle::MutexType::Lock lock2(fd_event->mutex);
	int op = (fd_event->event & ~event & ~EPOLLET) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
	int events = fd_event->event & ~event;
	fd_event->event = (Event)events; 
	lock2.unlock();
	
	epoll_event env ; 
	memset(&env , 0 , sizeof(env));
	env.events = events;
	env.data.ptr = fd_event;

	int ret = epoll_ctl(m_epfd,op,fd,&env);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"epoll_ctl no success errno : "<<errno 
								   <<" strerr : "<< strerror(errno);
		 return false;
	}
	if(event & READ){
		fd_event->read.reset();
	}
	if(event & WRITE){
		fd_event->write.reset();
	}
	--m_eventCount;
	return true;
}

bool IOManager::cancelEvent(int fd , Event event){
	if(fd >= (int)m_fds.size()){
		tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(fd);
		if(sts)sts->setAutoStop(false);
		return false;
	}
	MutexType::RDLock lock(m_mutex);
	EventHandle * fd_event = m_fds[fd];
	if(!(fd_event->event & event)){
		tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(fd);
		if(sts)sts->setAutoStop(false);
		return false;
	}
	lock.unlock();
	
	
	EventHandle::MutexType::Lock lock2(fd_event->mutex);
	//TADPOLE_LOG_INFO(g_logger) << fd_event->event;
	int op = (fd_event->event & ~event & ~EPOLLET) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
	int events = fd_event->event & ~event;
	fd_event->event = (Event)events; 
	lock2.unlock();
	
	epoll_event env ; 
	memset(&env , 0 , sizeof(env));
	env.events = events;
	env.data.ptr = fd_event;

	int ret = epoll_ctl(m_epfd,op,fd,&env);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"epoll_ctl no success errno : "<<errno 
								   <<" strerr : "<< strerror(errno);
		 return false;
	}
	fd_event->triggered(event);
	--m_eventCount;
	return true;
}

bool IOManager::cancelAllEvent(int fd){
	if(fd >= (int)m_fds.size()){
		tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(fd);
		if(sts)sts->setAutoStop(false);
		return false;
	}
	MutexType::RDLock lock(m_mutex);
	EventHandle * fd_event = m_fds[fd];
	lock.unlock();
	
	
	EventHandle::MutexType::Lock lock2(fd_event->mutex);
	if(!(fd_event->event & (READ|WRITE))){
		tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(fd);
		if(sts)sts->setAutoStop(false);
		return false; 
	}
	int op = EPOLL_CTL_DEL;
	int trievent = fd_event->event;
	fd_event->event = NONE; 
	lock2.unlock();
	
	epoll_event env ; 
	memset(&env , 0 , sizeof(env));
	env.events = (Event)trievent;
	env.data.ptr = fd_event;

	int ret = epoll_ctl(m_epfd,op,fd,&env);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"epoll_ctl no success errno : "<<errno 
								   <<" strerr : "<< strerror(errno);
		 return false;
	}
	fd_event->triggered((Event)trievent);
	if(trievent & READ)
		--m_eventCount;
	if(trievent & WRITE)
		--m_eventCount;
	return true;
}

void IOManager::tickle(){
	if(hasIdleThread()){
		uint8_t tick;
		write(m_tickles[1],&tick,1);
	}
}

void IOManager::idle(){
	static const int MAX_EVENT_SIZE = 256;
	epoll_event * events = new epoll_event[MAX_EVENT_SIZE];
	std::shared_ptr<epoll_event> deleter(events,[](epoll_event * ptr){
			delete[] ptr;
		});
	while(1){
		if(stopping()){
			break;
		}
		uint64_t timeout = getFirstTimerTimeOut();
		timeout = (timeout == ~0ull) ? 1000: timeout;
		int tm = timeout;
		int ret = epoll_wait(m_epfd,events,MAX_EVENT_SIZE,tm);
		if(ret < 0){
			if(errno == EINTR){
			}else {
				break;
			}
		}else if(ret == 0){
		}else {
			for(int i= 0 ; i < ret ; ++i){
				if(events[i].data.fd == m_tickles[0]){
					uint64_t buf; 
					while(0 < read(m_tickles[0],&buf,8));
					continue ; 
				}
				EventHandle * fd_event = (EventHandle*)events[i].data.ptr;
				int fd = fd_event->fd; 
				int ev = events[i].events;
				cancelEvent(fd,(Event)ev);
			}
		}
		
		std::vector<Timer::ptr> vec;
		getTimeOutTimer(vec);

		for(auto &it : vec){
			schedule(it->getCb());
		}

		Fiber::YieldToReady();
	}
}

void IOManager::timerTickle(){
	tickle();
}

bool IOManager::stopping(){
	return Scheduler::stopping() && m_eventCount == 0 ;
}

}
