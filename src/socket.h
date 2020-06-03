#ifndef __TADPOLE_SOCKET_H__
#define __TADPOLE_SOCKET_H__

#include "address.h"

namespace tadpole{

/**
 * @brief 套接字类
 */
class Socket{
public:
	/**
	 * @brief 类型定义，智能执政
	 */
	typedef std::shared_ptr<Socket> ptr;
	
	/**
	 * @brief 类型，方便识别
	 */
	enum type {
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	/**
	 * @brief 地址族，方便识别
	 */
	enum family {
		IPv4 = AF_INET,
		IPv6 = AF_INET6,
		Unix = AF_UNIX
	};
	
	/**
	 * @brief 构造函数
	 * @param[in] family 地址族
	 * @param[in] type 协议类型
	 * @param[in] protocal 协议
	 */
	Socket(int family ,int type , int protocal = 0);
	
	/**
	 * @brief 判断套接字是否有效
	 * @return ture 有效
	 */
	bool isValid(){
		return m_sock != -1;
	}
	/** 
	 * @brief 创建一个tcp套接字，ipv4版本
	 */
	static Socket::ptr CreateTcpSocket();

	/** 
	 * @brief 创建一个udp套接字，v4版本
	 */
	static Socket::ptr CreateUdpSocket();

	/** 
	 * @brief 创建一个tcp套接字，v6版本
	 */
	static Socket::ptr CreateTcp6Socket();

	/**
	 * @brief 创建一个udp套接字，v6版本
	 */
	static Socket::ptr CreateUdp6Socket();
	
	/** 
	 * @brief 获得套接字的读超时
	 */
	uint64_t getRecvTimeout();

	/**
	 * @brief 获得套接字的写超时
	 */
	uint64_t getSendTimeout();
	
	/**
	 * @brief 设置该文件描述符的读超时时间,是文件描述符多久没消息响应
	 * @param[in] tmout 超时时间，毫秒
	 */
	void setRecvTimeout(uint64_t tmout);

	
	/**
	 * @brief 设置该文件描述符的写超时时间,是文件描述符多久没消息响应
	 * @param[in] tmout 超时时间，毫秒
	 */
	void setSendTimeout(uint64_t tmout);

	/**
	 * @brief 设置套接字的一些属性
	 */
	int setOption(int level, int optname,const void *optval, socklen_t optlen);

	/**
	 * @brief 获得套接字的一些属性
	 */
	int getOption(int level, int optname, void *optval, socklen_t *optlen);
	
	/**
	 * @brief 使用该套接字绑定端口和地址
	 */
	bool bind(Address::ptr addr);

	/**
	 * @brief 开始监听
	 */
	bool listen(int maxconn = SOMAXCONN);

	/**
	 * @brief 接收新的连接
	 * @return 返回新的套接字对象
	 */
	Socket::ptr accept();

	/**
	 * @brief 连接函数
	 * @param[in] addr 地址类
	 */
	bool connect(Address::ptr addr);
	
	/**
	 * @brief 接收消息
	 * @param[in] buf 缓冲区
	 * @param[in] len buf长度
	 * @param[in] flag 标志位
	 */
	int recv(void * buf , size_t len,int flag = 0 );

	/**
	 * @brief udp 接收消息
	 * @param[in] buf 缓冲区
	 * @param[in] len buf长度
	 * @param[out] addr 用于接收对端的ip地址和端口
	 * @param[in] flag 标志位
	 */
	int recvFrom(void *buf , size_t len , Address::ptr addr , int flag = 0 );

	/**
	 * @brief 发送消息
	 * @param[in] buf 缓冲区
	 * @param[in] len buf长度
	 * @param[in] flag 标志位
	 */
	int send(void * buf , size_t len , int flags = 0);

	/**
	 * @brief udp 发送消息
	 * @param[in] buf 缓冲区
	 * @param[in] len buf长度
	 * @param[out] addr 目标ip地址
	 * @param[in] flag 标志位
	 */
	int sendTo(void * buf , size_t len , Address::ptr addr , int falgs = 0 );
		
	/**
	 * @brief 接收消息
	 * @param[in] iov iov数组结构，里面保存者buf指针和长度
	 * @param[in] flag 标志位
	 */
	int recv(const std::vector<iovec>& iov,int flag = 0 );

	/**
	 * @brief udp 接收消息
	 * @param[in] iov iov数组结构，里面保存者buf指针和长度
	 * @param[out] addr 用于保存对端的ip地址和端口
	 * @param[in] flag 标志位
	 */
	int recvFrom(const std::vector<iovec>& iov, Address::ptr addr , int flag = 0 );

	/**
	 * @brief 发送消息
	 * @param[in] iov iov数组结构，里面保存者buf指针和长度
	 * @param[in] flag 标志位
	 */
	int send(const std::vector<iovec> &vec, int flags = 0);

	/**
	 * @brief udp 发送消息
	 * @param[in] iov iov数组结构，里面保存者buf指针和长度
	 * @param[in] addr 目标ip地址和端口
	 * @param[in] flag 标志位
	 */
	int sendTo(const std::vector<iovec>& vec, Address::ptr addr , int falgs = 0 );

	/**
	 * @brief 关闭套接字
	 */
	int close();
	/**
	 * @brief 获得本套接字的地址
	 */
	Address::ptr getLocalAddress();

	/**
	 * @brief 获得对端套接字的地址
	 */
	Address::ptr getRemoteAddress();
private:
	/**
	 * @brief 构造函数，给accept用
	 */
	Socket(int family , int type , int protocal, int fd);
private:
	//文件描述符
	int m_sock ;
	//地址族协议
	int m_family ;
	//协议类型
	int m_type ;
	//协议
	int m_protocal ; 
	//本地地址
	Address::ptr m_localAddress; 
	//对端地址
	Address::ptr m_remoteAddress; 
};

}

#endif 
