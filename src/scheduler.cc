#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

static thread_local Scheduler * t_scheduler = nullptr;

Scheduler* Scheduler::GetCurScheduler(){
	if(t_scheduler){
		return t_scheduler;
	}
	return nullptr;
}

void Scheduler::callerBegin(){
	if(m_rootFiber){
		m_rootFiber->call();
	}
}

Scheduler::Scheduler(uint32_t threadcount
					,bool usercaller ,const std::string & name )
					:m_threadCount(threadcount)
					,m_name(name){
	t_scheduler = this;
	if(usercaller){
		Fiber::GetCurFiber();
		--m_threadCount;

		m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run,this)));
		m_rootThreadId = GetThreadId();

		t_scheduler = this; 
		set_hook_enable(true);
	}else {
		set_hook_enable(false);
		m_rootThreadId = -1;
	}						
}

void Scheduler::start(){
	m_threads.resize(m_threadCount);
	
	MutexType::Lock lock();
	for(size_t i = 0 ; i < m_threadCount;++i ){
		m_threads[i].reset(new Thread(std::bind(&Scheduler::run,this),
		"thread"+std::to_string(i)));
	}
}

void Scheduler::tickle(){
	TADPOLE_LOG_INFO(g_logger) << "tickle";
}

void Scheduler::idle(){
	TADPOLE_LOG_INFO(g_logger) << "idle";
}

bool Scheduler::stopping(){
	MutexType::Lock lock(m_mutex);
	bool empty = m_fiberQueue.empty();
	lock.unlock();
	return m_isStopping && m_activeThreadCount == 0 && empty;
}

void Scheduler::stop(){
	m_isStopping = true;
	for(uint32_t i = 0 ; i < m_threadCount ; ++i){
		tickle();
	}
	if(m_rootThreadId != -1){
		tickle();
	}
	
	for(uint32_t i = 0 ; i < m_threadCount ; ++i){
		m_threads[i]->join();
	}
}

Scheduler::~Scheduler(){
	//使用调用者线程
	callerBegin();
	t_scheduler = nullptr;
}

void Scheduler::run(){
	set_hook_enable(true);
	t_scheduler = this;

	if(GetThreadId() != (uint32_t)m_rootThreadId){
		Fiber::GetCurFiber();
	}

	Fiber::ptr main_fiber;
	Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle,this)));

	while(1){
		if(stopping()){
			break;
		}
		ThreadAndFiber tf; 
		MutexType::Lock lock(m_mutex);
		auto it = m_fiberQueue.begin();
		while(it != m_fiberQueue.end()){
			if(-1 == it->thread){
				tf = *it;
				m_fiberQueue.erase(it);
				break;
			}

			if(GetThreadId() == (uint32_t)it->thread){
				tf = *it ; 
				m_fiberQueue.erase(it);
				break;
			}else {
				tickle();
				++it;
			}
		}
		lock.unlock();
		
		if(tf.cb){
			main_fiber.reset(new Fiber(tf.cb));
		}else if(tf.fiber){
			main_fiber = tf.fiber;
		}else {
			++m_idleThreadCount;
			idle_fiber->swapIn();
			--m_idleThreadCount;
			if(idle_fiber->getState() == Fiber::TERM || 
			   idle_fiber->getState() == Fiber::EXCEPT){
				break;   
			}
			continue; 
		}
		tf.reset();
		
		++m_activeThreadCount;
		main_fiber->swapIn();
		--m_activeThreadCount;
		if(main_fiber->getState() == Fiber::READY ||
		   main_fiber->getState() == Fiber::INIT){
			schedule(&main_fiber);
		}
		main_fiber.reset();
	}
}
}
