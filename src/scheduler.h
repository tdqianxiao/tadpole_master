#ifndef __TADPOLE_SCHEDULER_H__
#define __TADPOLE_SCHEDULER_H__

#include <memory>
#include <vector>
#include <list>
#include <atomic>

#include "src/thread.h"
#include "src/mutex.h"
#include "src/fiber.h"

namespace tadpole{

/**
 * @brief 协程调度器
 */
class Scheduler:public std::enable_shared_from_this<Scheduler>{
public:
	/**
	 * @brief 定义协程调度器智能指针
	 */
	typedef std::shared_ptr<Scheduler> ptr;
	
	/**
	 * @brief 锁类型
	 */
	typedef Mutex MutexType;
public:
	
	/**
	 * @brief 构造函数
	 * @param[in] threadcount 该调度器使用的线程数量
	 * @param[in] usercaller 是否使用调用者线程
	 * @param[in] name 调度器名称
	 */
	Scheduler(uint32_t threadcount,bool usercaller = false,const std::string & name = "");

	/**
	 * @brief 析构函数
	 */
	virtual ~Scheduler();

private:
	/**
	 * @brief 内部结构体，用来给协程队列当元素的
	 */
	struct ThreadAndFiber{
		//线程
		int thread;
		//协程
		Fiber::ptr fiber;
		//回调函数
		std::function<void()> cb;
		
		/**
		 * @brief 构造函数
		 * @param[in] f 协程
		 * @param[in] th 指定线程
		 */
		ThreadAndFiber(Fiber::ptr f,int th = -1)
			:thread(th){
			fiber = f; 
		}
			
		/**
		 * @brief 构造函数
		 * @param[in] f 函数对象
		 * @param[in] th 指定线程
		 */
		ThreadAndFiber(std::function<void()> f,int th = -1)
			:thread(th){
			cb = f; 
		}
	
		/**
		 * @brief 构造函数
		 */	
		ThreadAndFiber()
			:thread(-1)
			,fiber(nullptr)
			,cb(nullptr){
		}

		/**
		 * @brief 构造函数
		 * @param[in] pf 协程对象智能指针的指针，用于交换原来的协程
		 * @param[in] th 指定线程
		 */	
		ThreadAndFiber(Fiber::ptr * pf,int th = -1)
			:thread(th){
			fiber.swap(*pf);
		}
			
		/**
		 * @brief 构造函数
		 * @param[in] pf 函数对象的指针，用于交换原来的函数对象
		 * @param[in] th 指定线程
		 */	
		ThreadAndFiber(std::function<void()> *pf,int th = -1)
			:thread(th){
			cb.swap(*pf);
		}

		/**
		 * @brief 重置该结构
		 */
		void reset(){
			thread = -1;
			fiber = nullptr;
			cb = nullptr;
		}
	};
public:

	/**
	 * @brief 往协程队列中添加函数或协程
	 * @param[in] 函数或协程
	 * @param[in] 指定执行线程
	 */
	template <class Func>
	void schedule(Func fun,int thread = -1){
		ThreadAndFiber fb(fun,thread);
		if(fb.fiber || fb.cb){
			MutexType::Lock lock(m_mutex);
			if(m_fiberQueue.empty()){
				tickle();
			}
			m_fiberQueue.push_back(fb);
		}
	}
	
	/**
	 * @brief 批量往协程队列中添加函数或协程,所有线程池线程均可执行
	 * @param[in] begin 迭代器
	 * @param[in] end 迭代器
	 */
	template <class Iter>
	void schedule(Iter begin,Iter end){
		bool is_tickle = false;
		MutexType::Lock lock(m_mutex);
		for(; begin != end; ++begin){
			ThreadAndFiber fb(*begin);
			if(fb.fiber || fb.cb){
				is_tickle = is_tickle || m_fiberQueue.empty();
				m_fiberQueue.push_back(fb);
			}
		}
	}
public:
	/**
	 * @brief 是否有空闲协线程
	 */
	bool hasIdleThread(){return m_idleThreadCount > 0;}

	/**
	 * @brief 通知一下idle里面阻塞的协程
	 */
	virtual void tickle();

	/**
	 * @brief 线程无任务将执行的空闲函数
	 */
	virtual void idle();

	/**
	 * @brief 判断调度器是否停止
	 */
	virtual bool stopping();
	
	/**
	 * @brief 获得当前线程的调度器
	 */
	static Scheduler* GetCurScheduler();

	/**
	 * @brief 调度器开始执行
	 */
	void start();

	/**
	 * @brief 调度器停止,会等待任务执行完
	 */
	void stop();
	
	/**
	 * @brief 若使用调用者线程，调用此函数调用者线程才会开始被调度
	 */
	void callerBegin();
private:

	/**
	 * @brief 线程调度器执行入口
	 */
	void run();
private:
	//是否停止
	bool m_isStopping = false;
	//线程数量
	uint32_t m_threadCount = 0 ;
	//调度器名称
	std::string m_name ; 
	//调用者线程id,若未使用调用者线程则未-1
	int m_rootThreadId = 0;
	//调用者主协程
	Fiber::ptr m_rootFiber;
	//互斥锁	
	MutexType m_mutex;
	//线程池
	std::vector<Thread::ptr> m_threads;
	//活动线程数量
	std::atomic<uint32_t> m_activeThreadCount = {0};
	//空闲线程数量
	std::atomic<uint32_t> m_idleThreadCount = {0};
	//协程队列
	std::list<ThreadAndFiber> m_fiberQueue;
};

}

#endif 
