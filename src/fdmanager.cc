#include "src/fdmanager.h"
#include "src/hook.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace tadpole{
FdSts::FdSts(int fd)
	:m_fd(fd)
	,m_isInit(false)
	,m_isSocket(false)
	,m_sysNonblock(false)
	,m_userNonblock(false)
	,m_isClose(false)
	,m_recvTimeOut(-1)
	,m_sendTimeOut(-1){
	init();
}
FdSts::~FdSts(){
}

bool FdSts::init(){
	struct stat fd_stat = {}; 
	if(-1 != fstat(m_fd,&fd_stat)){
		m_isInit = true;
		m_isSocket = S_ISSOCK(fd_stat.st_mode);
	}
	if(m_isSocket){
		//如果是socket直接设成非阻塞
		int flag = fcntl_f(m_fd,F_GETFL,0);
		flag |= O_NONBLOCK;
		fcntl_f(m_fd,F_SETFL,flag);
		m_sysNonblock = true;
	}
	m_isClose = false;
	m_userNonblock = false;
	return m_isInit;
}

uint64_t FdSts::getTimeOut(int type){
	if(type == SO_RCVTIMEO){
		return m_recvTimeOut;
	}else{
		return m_sendTimeOut;
	}
}
void FdSts::setTimeOut(int type , uint64_t timeout){
	if(type == SO_RCVTIMEO){
		m_recvTimeOut = timeout;
	}else{
		m_sendTimeOut = timeout;
	}
}

FdManager::FdManager(){
	//初始化大小为64
	m_fdctxs.resize(64);
}

FdSts::ptr FdManager::get(int fd , bool auto_create ){
	MutexType::RDLock lock(m_mutex);
	if(fd >= (int)m_fdctxs.size()){
		if(!auto_create){
			return nullptr;
		}else {
			lock.unlock();
			FdSts::ptr fd_ctx(new FdSts(fd));
			MutexType::WRLock lock2(m_mutex);
			m_fdctxs.resize(fd*1.5);
			m_fdctxs[fd] = fd_ctx;
			return fd_ctx;
		}
	}else {
		if(m_fdctxs[fd] || !auto_create){
			return m_fdctxs[fd];
		}
	}

	lock.unlock();
	FdSts::ptr fd_ctx(new FdSts(fd));
	MutexType::WRLock lock2(m_mutex);
	m_fdctxs[fd] = fd_ctx;
	return fd_ctx;
}

void FdManager::del(int fd){
	MutexType::WRLock lock(m_mutex);
	if(fd >= (int)m_fdctxs.size()){
		return;
	}else {
		m_fdctxs[fd].reset();
	}
}
}
