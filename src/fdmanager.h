#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include <vector>
#include <memory>

#include "src/mutex.h"
#include "src/singleton.h"
namespace tadpole{

/**
 * @brief 文件描述符状态类
 */
class FdSts:public std::enable_shared_from_this<FdSts>{
public:
	/**
	 * @brief 类型定义，文件描述符状态类智能执政
	 */
	typedef std::shared_ptr<FdSts> ptr;
public:
	/**
	 * @brief 构造函数
	 */
	FdSts(int fd);

	/**
	 * @brief 析构函数
	 */
	~FdSts();

	/**
	 * @brief 初始化函数
	 */
	bool init();
	
	/**
	 * @brief 返回该文件描述符是否初始化
	 */
	bool isInit()const {return m_isInit;}
	
	/**
	 * @brief 比如说recv是，获得取消时是否退出来
	 */
	bool isAutoStop()const{return m_isAutoStop;}
	
	/**
	 * @brief 设置取消时是否退出来
	 */
	void setAutoStop(bool isstop){m_isAutoStop = isstop;}
	/**
	 * @brief 返回该文件描述符是否为socket
	 */
	bool isSocket()const {return m_isSocket;}

	/**
	 * @brief 返回该文件描述符是否关闭
	 */
	bool isClose()const {return m_isClose;}
	
	/**
	 * @brief 获得系统是否设置了nonblock
	 */
	bool getSysNonblock()const {return m_sysNonblock;}

	/**
	 * @brief 设置系统nonblock属性
	 * @param[in] sys 设置是否nonblock
	 */
	void setSysNonblock(bool sys){ m_sysNonblock = sys ; }

	/**
	 * @brief 获得用户是否设置了nonblock 属性
	 */
	bool getUserNonblock()const {return m_userNonblock;}

	/**
	 * @brief 这是用户nonblock属性
	 * @param[in] user 设置是否nonblock
	 */
	void setUserNonblock(bool user){m_userNonblock = user;}
	
	/**
	 * @brief 获得该文件描述符文件超时时间，默认为-1，
	 * @param[in] type 类型SO_RCVTIMEO为读超时，SO_SNDTIMEO为写超时
	 */
	uint64_t getTimeOut(int type);

	/**
	 * @brief 设置文件描述符超时时间
	 * @param[in] type 类型
	 * @param[in] timeout 超时时间，毫秒
	 */
	void  setTimeOut(int type , uint64_t timeout);

private:
	//文件描述符
	int m_fd = -1;
	//是否初始化
	bool m_isInit = 0;
	//是否为socket
	bool m_isSocket = 0 ; 
	//系统是否设置了nonblock
	bool m_sysNonblock = 0 ;
	//用户是否设置了nonblock
	bool m_userNonblock = 0;
	//文件描述符是否关闭
	bool m_isClose = 0 ; 
	//取消事件是是否退出来
	bool m_isAutoStop = false;
	//读超时
	uint64_t m_recvTimeOut = -1 ; 
	//写超时
	uint64_t m_sendTimeOut = -1 ; 
};

/**
 * @brief 文件描述符管理类
 */
class FdManager{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<FdManager> ptr;

	/**
	 * @brief 类型定义，锁类型
	 */
	typedef RWMutex MutexType;

	/**
	 * @brief 构造函数
	 */
	FdManager();

	/**
	 * @brief 获得文件描述符状态
	 * @brief[in] fd 文件描述符
	 * @brief[in] autoCreate 是否主动创建该文件描述符,默认false;
	 */
	FdSts::ptr get(int fd , bool autoCreate = false );
	/**
	 * @brief 删除文件描述符
	 */
	void del(int fd);
	
private:
	// 锁
	MutexType m_mutex;
	// 文件描述符状态数组,1.5倍增长
	std::vector<FdSts::ptr> m_fdctxs;
};

/**
 * @brief 把文件描述符管理类定义为单例类
 */
typedef Singleton<FdManager> FdMgr;

}

#endif 
