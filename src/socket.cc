#include "socket.h"
#include "fdmanager.h"
#include "macro.h"
#include "log.h"
#include <netinet/tcp.h>

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

Socket::ptr Socket::CreateTcpSocket(){
	return Socket::ptr(new Socket(IPv4,TCP,0)); 
}

Socket::ptr Socket::CreateUdpSocket(){
	return Socket::ptr(new Socket(IPv4,UDP,0));
}

Socket::ptr Socket::CreateTcp6Socket(){
	return Socket::ptr(new Socket(IPv6,TCP,0)); 
}

Socket::ptr Socket::CreateUdp6Socket(){
	return Socket::ptr(new Socket(IPv6,UDP,0));
}

Socket::Socket(int family ,int type , int protocal)
	:m_sock(-1)
	,m_family(family)
	,m_type(type)
	,m_protocal(protocal){
	m_sock = socket(m_family,m_type,m_protocal);
	int val = 1; 
	socklen_t n = sizeof(val);
	setOption(IPPROTO_TCP,TCP_NODELAY,&val,n);
}

Socket::Socket(int family , int type , int protocal , int fd)
	:m_sock(fd)
	,m_family(family)
	,m_type(protocal)
	,m_protocal(protocal){
	int val = 1;
	socklen_t n = sizeof(val);
	setOption(IPPROTO_TCP,TCP_NODELAY,&val,n);
}

int Socket::close(){
	if(isValid()){
		return ::close(m_sock);
	}
	return -1;
}

int Socket::setOption(int level, int optname,
                      const void *optval, socklen_t optlen){
	return setsockopt(m_sock,level,optname,optval,optlen);	
}

int Socket::getOption(int level, int optname,
                      void *optval, socklen_t *optlen){
	return getsockopt(m_sock,level,optname,optval,optlen);
}

uint64_t Socket::getRecvTimeout(){
	FdSts::ptr sts = FdMgr::GetInstance()->get(m_sock);
	if(sts){
		return sts->getTimeOut(SO_RCVTIMEO);
	}
	return -1;
}

uint64_t Socket::getSendTimeout(){
	FdSts::ptr sts = FdMgr::GetInstance()->get(m_sock);
	if(sts){
		return sts->getTimeOut(SO_SNDTIMEO);
	}
	return -1; 
}

bool Socket::bind(Address::ptr addr){
	if(isValid()){
		TADPOLE_ASSERT(addr->getAddr()->sa_family == m_family);
		int ret = ::bind(m_sock,addr->getAddr(), addr->getAddrLen());
		if(ret == -1){
			TADPOLE_LOG_ERROR(g_logger) << "bind("<<addr->toString()
										<< ") non success ! errno :"
										<< errno << " strerr : "
										<< strerror(errno);
			return false; 
		}
		return true;
	}		
	return false;
}

bool Socket::listen(int maxconn){
	if(isValid()){
		int ret = ::listen(m_sock,maxconn);
		if(ret == -1){
			TADPOLE_LOG_ERROR(g_logger) << "listen(fd:"<<m_sock
										<< ") non success ! errno :"
										<< errno << " strerr : "
										<< strerror(errno);
			return false; 
		}
		return true;
	}
	return false;
}

Socket::ptr Socket::accept(){
	if(isValid()){
		Socket::ptr sock;
		int fd = ::accept(m_sock,nullptr,nullptr);
		if(fd == -1){
			TADPOLE_LOG_ERROR(g_logger) << "accept() non success ! errno :"
										<< errno << " strerr : "
										<< strerror(errno);
			return nullptr; 
		}
		sock.reset(new Socket(m_family,m_type,m_protocal,fd));
		return sock;
	}
	return nullptr;
}

bool Socket::connect(Address::ptr addr){
	if(isValid()){
		int ret = ::connect(m_sock,addr->getAddr(),addr->getAddrLen());
		if(ret != 0){
			TADPOLE_LOG_ERROR(g_logger) << "connect("<<addr->toString()
										<< ") non success ! errno :"
										<< errno << " strerr : "
										<< strerror(errno);
			return false; 	
		}
		return true;
	}
	return false;
}

int Socket::recv(void * buf , size_t len,int flag ){
	return ::recv(m_sock,buf,len,flag);
}

int Socket::recvFrom(void *buf , size_t len , Address::ptr addr , int flag){
	return recvfrom(m_sock,buf,len,flag,addr->getAddr(),nullptr);
}

int Socket::send(void * buf , size_t len , int flags){
	return ::send(m_sock,buf,len,flags);
}

int Socket::sendTo(void * buf , size_t len ,Address::ptr addr,int flags){
	return ::sendto(m_sock,buf,len,flags,addr->getAddr(),addr->getAddrLen());
}

int Socket::recv(iovec *iov,int flag ){
	return ::recv(m_sock,iov->iov_base,iov->iov_len,flag);
}

int Socket::recvFrom(iovec * iov, Address::ptr addr , int flag){
	return recvfrom(m_sock,iov->iov_base,iov->iov_len,flag,addr->getAddr(),nullptr);
}

int Socket::send(iovec * iov, int flags){
	return ::send(m_sock,iov->iov_base,iov->iov_len,flags);
}

int Socket::sendTo(iovec * iov,Address::ptr addr,int flags){
	return ::sendto(m_sock,iov->iov_base,iov->iov_len,flags,addr->getAddr(),addr->getAddrLen());
}

Address::ptr Socket::getLocalAddress(){
	if(m_localAddress){
		return m_localAddress;
	}
	if(isValid()){
		Address::ptr addr;
		if(m_family == IPv4){
			addr.reset(new IPv4Address());
		}else if(m_family == IPv6){
			addr.reset(new IPv6Address());
		}else if(m_family == Unix){
			addr.reset(new UnixAddress());
		}else {
			addr.reset(new UnKnowAddress());
		}
		socklen_t len; 
		if(0 != getsockname(m_sock,addr->getAddr(),&len)){
			TADPOLE_LOG_ERROR(g_logger) << "getLocalAddress("<<addr->toString()
										<< ") non success ! errno :"
										<< errno << " strerr : "
										<< strerror(errno);
			return nullptr;
		}
		if(addr->getAddr()->sa_family == Unix){
			std::dynamic_pointer_cast<UnixAddress>(addr)->setLen(len);
		}
		m_localAddress = addr;
		return addr;
	}
	return nullptr;
}

Address::ptr Socket::getRemoteAddress(){
	if(m_remoteAddress){
		return m_remoteAddress;
	}
	if(isValid()){
		Address::ptr addr;
		if(m_family == IPv4){
			addr.reset(new IPv4Address());
		}else if(m_family == IPv6){
			addr.reset(new IPv6Address());
		}else if(m_family == Unix){
			addr.reset(new UnixAddress());
		}else {
			addr.reset(new UnKnowAddress());
		}
		socklen_t len; 
		if(0 != getpeername(m_sock,addr->getAddr(),&len)){
			TADPOLE_LOG_ERROR(g_logger) << "getRemoteAddress("<<addr->toString()
										<< ") non success ! errno :"
										<< errno << " strerr : "
										<< strerror(errno);
			return nullptr;
		}
		if(addr->getAddr()->sa_family == Unix){
			std::dynamic_pointer_cast<UnixAddress>(addr)->setLen(len);
		}
		m_remoteAddress = addr;
		return addr;
	}
	return nullptr;
}


}
