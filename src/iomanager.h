#ifndef __TADPOLE_IOMANAGER_H__
#define __TADPOLE_IOMANAGER_H__

#include <unistd.h>
#include <sys/epoll.h>

#include "src/scheduler.h"
#include "src/timer.h"

namespace tadpole{

/**
 * @brief IO协程调度器，继承调度器，定时器
 */
class IOManager :public Scheduler,public TimerMgr{
public:
	/**
	 * @brief 类型定义，智能执政
	 */
	typedef std::shared_ptr<IOManager> ptr;

	/**
	 * @brief 读写锁类型定义
	 */
	typedef RWMutex MutexType ; 

	/**
	 * @brief 构造函数
	 * @param[in] threadcount 线程数量
	 * @param[in] usercaller 是否使用调用者线程
	 * @param[in] name 调度器名称
	 */
	IOManager(uint32_t threadcount , bool usercaller = false,
			  const std::string & name = "");
	/**
	 * @brief 析构函数
	 */
	~IOManager();

	/**
	 * @brief 自定义事件枚举
	 * @brief READ 读事件
	 * @brief WRITE 写事件
	 */
	enum Event{
		NONE = 0,
		READ = EPOLLIN,
		WRITE = EPOLLOUT,
	};
private:
	/**
	 * @brief 协程结构
	 */
	struct FiberOrFunc{
		FiberOrFunc()
			:iom(nullptr)
			,cb(nullptr)
			,fiber(nullptr){
		}
		IOManager * iom;
		std::function<void()> cb;
		Fiber::ptr fiber ;
		
		/**
		 * @brief 重置该结构
		 */
		void reset(){
			iom = nullptr;
			cb = nullptr;
			fiber = nullptr;
		}
	};
	
	/**
	 * @brief 事件句柄，有读事件或写事件
	 */
	struct EventHandle{
		typedef Mutex MutexType;
		EventHandle(int s)
			:fd(s)
			,event(NONE){
		}
		/**
		 * @breif 触发事件
		 */
		void triggered(Event ev);
		
		//文件描述符
		int fd = -1 ;
		//事件类型
		Event event;
		//读事件
		FiberOrFunc read;
		//写事件
		FiberOrFunc write; 
		//互斥锁
		MutexType mutex;
	};
public:
	/**
	 * @brief 添加事件
	 * @param[in] fd 文件描述符
	 * @param[in] event 需要监听的事件
	 * @param[in] cb 回调函数,若微空则把当前协程加入事件
	 * @return 返回true事件添加成功
	 */
	bool addEvent(int fd,Event event,std::function<void()> cb = nullptr);
	
	/**
	 * @brief 删除事件
	 * @param[in] fd 文件描述符
	 * @param[in] event 需要删除的事件
	 * @return 返回true事件删除成功
	 */
	bool delEvent(int fd,Event event);
	
	/**
	 * @brief 取消事件,会强制触发事件
	 * @param[in] fd 文件描述符
	 * @param[in] event 需要取消的事件
	 * @return 返回true事件取消成功
	 */
	bool cancelEvent(int fd,Event event);

	/**
	 * @brief 取消一个文件描述符所有事件
	 * @param[in] 文件描述符
	 * @return 返回true事件取消成功
	 */
	bool cancelAllEvent(int fd);

	/**
	 * @brief 重写有任务通知线程函数
	 */
	void tickle()override ; 

	/**
	 * @brief 重写空闲协程执行函数
	 */
	void idle()override ;

	/**
	 * @brief 重写是否停止函数
	 * @return 返回true调度器停止调度
	 */
	bool stopping()override ;

	/**
	 * @brief 重写,定时器通知有任务
	 */
	void timerTickle()override;

	/**
	 * @brief 重置事件列表大小
	 */
	void resizeFdsSize(int size);
	
	/**
	 * @brief 获得当前io协程调度器
	 * @return 没有则返回空
	 */
	static IOManager * GetCurIOM();
private:
	//epoll根节点文件描述符
	int m_epfd;
	//事件列表
	std::vector<EventHandle *> m_fds;
	//事件多少
	std::atomic<uint32_t> m_eventCount = {0};
	//管道，用于通知线程
	int m_tickles[2];
	//锁
	MutexType m_mutex;
};
}

#endif 
