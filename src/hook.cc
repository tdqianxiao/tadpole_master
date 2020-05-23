#include "hook.h"
#include <dlfcn.h>
#include <stdarg.h>

#include "config.h"
#include "log.h"
#include "fiber.h"
#include "iomanager.h"
#include "fdmanager.h"
#include "macro.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

namespace tadpole{

static thread_local bool t_is_hook = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
	is_inited = true;
}

struct _HookIniter {
    _HookIniter() {
        hook_init();
    };
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_is_hook;
}

void set_hook_enable(bool flag) {
    t_is_hook = flag;
}

}

struct timer_info{
	int cancelled = 0;
};

/**
 * @brief 模板方法，io操作
 * @param[in] fd 文件描述符
 * @param[in] func 原来的系统函数
 * @param[in] hook_fun_name hook的函数名
 * @param[in] event 读写事件
 * @param[in] timeout_so 超时时间
 * @param[in] args 原函数除fd其他的参数
 */
template <class OldFun , class ... Args>
static ssize_t do_io(int fd , OldFun func ,const char * hook_fun_name ,
			 uint32_t event , int timeout_so , Args&&... args){
	TADPOLE_LOG_INFO(g_logger) << "do_io";
	//没hook执行原函数
	if(!tadpole::t_is_hook){
		return func(fd,std::forward<Args>(args)...);
	} 
	//保存该文件描述符的状态
	tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(fd);
	if(!sts){
		func(fd,std::forward<Args>(args)...);
	}
	
	//如果关闭errno置为EBADF
	if(sts->isClose()){
		errno = EBADF;
		return -1;
	}
	
	//若不为socket或没有系统设置非阻塞，执行原函数
	if(!sts->isSocket() || sts->getUserNonblock()){
		return func(fd,std::forward<Args>(args)...);
	}
	
	//获得该文件描述符的超时时间
	uint64_t to = sts->getTimeOut(timeout_so);
	//用于条件定时器
	std::shared_ptr <timer_info> tinfo(new timer_info);
retry:
	ssize_t n = func(fd,std::forward<Args>(args)...);
	//有超时时间的函数，如果超时会继续重复执行
	while(n == -1 && errno == EINTR){	
		n = func(fd,std::forward<Args>(args)...);
	}
	//重复读空buf,直接添加事件，切换协程
	if(n == -1 && errno == EAGAIN) {
		tadpole::IOManager* iom = tadpole::IOManager::GetCurIOM();
		tadpole::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

		if(to != (uint64_t)-1) {
			//添加条件定时器，若在延时期间未触发事件直接将事件取消
            timer = iom->addCondTimer(fd, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (tadpole::IOManager::Event)(event));
            }, winfo);
        }
		
		//添加事件,成功直接yieldtohold,失败退出吗,取消定时器,直接返回
        int rt = iom->addEvent(fd, (tadpole::IOManager::Event)(event));
        if(!rt) {
            TADPOLE_LOG_INFO(g_logger)<< hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            tadpole::Fiber::YieldToHold();
			//触发事件回来之后，取消定时器
            if(timer) {
                timer->cancel();
            }
			//如果取消了，直接设置errno，返回-1
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
			//直接回到上面继续io操作
            goto retry;
        }
    }
   	return n ;  
}


extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX


unsigned int sleep(unsigned int seconds) {
	if(!tadpole::t_is_hook){
		return sleep_f(seconds);
	}

	tadpole::Fiber::ptr fiber = tadpole::Fiber::GetCurFiber();
	tadpole::IOManager* iom = tadpole::IOManager::GetCurIOM();
	iom->addTimer(seconds*1000,std::bind((void(tadpole::Scheduler::*)(tadpole::Fiber::ptr, int))&tadpole::IOManager::schedule ,iom, fiber,-1));
	tadpole::Fiber::YieldToHold();
	return 0 ; 
}

int usleep(useconds_t usec){
	if(!tadpole::t_is_hook){
		return usleep_f(usec);
	}
	tadpole::Fiber::ptr fiber = tadpole::Fiber::GetCurFiber();
	tadpole::IOManager* iom = tadpole::IOManager::GetCurIOM();
	iom->addTimer(usec/1000,std::bind((void(tadpole::Scheduler::*)(tadpole::Fiber::ptr, int))&tadpole::IOManager::schedule ,iom, fiber,-1));
	tadpole::Fiber::YieldToHold();
	return 0 ; 
}

int nanosleep(const struct timespec * req , struct timespec * rem) {
	if(!tadpole::t_is_hook){
		return nanosleep(req,rem);
	}

	tadpole::Fiber::ptr fiber = tadpole::Fiber::GetCurFiber();
	tadpole::IOManager* iom = tadpole::IOManager::GetCurIOM();
	iom->addTimer(req->tv_sec*1000 + req->tv_nsec /(1000*1000),std::bind((void(tadpole::Scheduler::*)(tadpole::Fiber::ptr, int thread))&tadpole::IOManager::schedule ,iom, fiber,-1));
	tadpole::Fiber::YieldToHold();
	return 0 ; 
}

ssize_t read(int fd, void *buf, size_t count){
	return do_io(fd,read_f,"read",tadpole::IOManager::READ,SO_RCVTIMEO,buf,count);
}

int socket(int domain, int type, int protocol){
	if(!tadpole::t_is_hook){
		return socket_f(domain,type,protocol);
	}
	int fd = socket_f(domain,type,protocol);
	if(fd == -1)
		return -1;
	tadpole::FdMgr::GetInstance()->get(fd,true);
	return fd;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
	int cfd =  do_io(sockfd,accept_f,"accept",tadpole::IOManager::READ,SO_RCVTIMEO,addr,addrlen);
	if(cfd == -1){
		return -1;
	}
	tadpole::FdMgr::GetInstance()->get(cfd,true);
	return cfd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
	return connect_f(sockfd,addr,addrlen);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt){
	return do_io(fd,readv_f,"readv",tadpole::IOManager::READ,SO_RCVTIMEO,iov,iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags){
	return do_io(sockfd,recv_f,"recv",tadpole::IOManager::READ,SO_RCVTIMEO,buf,len,flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen){
	return do_io(sockfd,recvfrom_f,"recvfrom",tadpole::IOManager::READ,SO_RCVTIMEO,buf,len,flags,src_addr,addrlen);						
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){
	return do_io(sockfd,recvmsg_f,"recvmsg",tadpole::IOManager::READ,SO_RCVTIMEO,msg,flags);
}


ssize_t send(int sockfd, const void *buf, size_t len, int flags){
	return do_io(sockfd,send_f,"send",tadpole::IOManager::WRITE,SO_SNDTIMEO,buf,len,flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen){
	return do_io(sockfd,sendto_f,"sendto",tadpole::IOManager::WRITE,SO_SNDTIMEO,
				 buf,len,flags,dest_addr,addrlen);					  
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags){
	return do_io(sockfd,sendmsg_f,"sendmsg",tadpole::IOManager::WRITE,SO_SNDTIMEO,
				 msg,flags);
}

ssize_t write(int fd, const void *buf, size_t count){
	return do_io(fd,write_f,"write",tadpole::IOManager::WRITE,SO_SNDTIMEO,buf,count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt){
	return do_io(fd,writev_f,"writev",tadpole::IOManager::WRITE,SO_SNDTIMEO,iov,iovcnt);
}

int close(int fd){
	if(!tadpole::t_is_hook){
		return close(fd);
	}
	tadpole::FdSts::ptr sts = tadpole::FdMgr::GetInstance()->get(fd);
	if(sts){
		auto iom = tadpole::IOManager::GetCurIOM();
		iom->cancelAllEvent(fd);
		tadpole::FdMgr::GetInstance()->del(fd);
	}
	return close(fd);
}
int fcntl(int fd, int cmd, ... ){
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                tadpole::FdSts::ptr sts = tadpole::FdMgr::GetInstance()->get(fd);
                if(!sts || sts->isClose() || !sts->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                sts->setUserNonblock(arg & O_NONBLOCK);
                if(sts->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                tadpole::FdSts::ptr sts = tadpole::FdMgr::GetInstance()->get(fd);
                if(!sts || sts->isClose() || !sts->isSocket()) {
                    return arg;
                }
                if(sts->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, long unsigned int request,...){
	va_list va ; 
	va_start(va,request);
	void * arg = va_arg(va,void*);
	va_end(va);
	return ioctl_f(d,request,arg);
}

int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen){
	return getsockopt_f(sockfd,level,optname,optval,optlen);			   
}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen){
    if(!tadpole::t_is_hook) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            tadpole::FdSts::ptr sts = tadpole::FdMgr::GetInstance()->get(sockfd);
            if(sts) {
                const timeval* v = (const timeval*)optval;
                sts->setTimeOut(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}
