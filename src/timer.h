#ifndef __TADPOLE_TIMER_H__
#define __TADPOLE_TIMER_H__

#include <memory>
#include <vector>
#include <set>
#include "mutex.h"

namespace tadpole{
//声明
class TimerMgr;

/**
 * @brief 定时器类
 */
class Timer:public std::enable_shared_from_this<Timer>{
friend class TimerMgr;
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<Timer> ptr;
public:
	/**
	 * @brief 构造函数
	 * @param[in] ms 时间间隔
	 * @param[in] cb 回调函数
	 * @param[in] mgr 该定时器的管理类指针
	 * @param[in] isloop 是否循环
	 */
	Timer(uint64_t ms,std::function<void()> cb,TimerMgr * mgr,bool isloop = false);

	/**
	 * @brief 用于创建比较对象的构造函数
	 */
	Timer(uint64_t next);
	
	/**
	 * @brief 获得回调函数
	 */
	std::function<void()> getCb()const {return m_cb;};
	
	/**
	 * @brief 简单的检查一下时间是否被修改
	 */
	bool checkTime();
	
	/**
	 * @brief 取消该定时器
	 */
	void cancel();

	/**
	 * @brief 重置该定时器
	 */
	void reset();

	/**
	 * @brief 直接刷新定时器
	 */
	void reflush();
private:
	/**
	 * @brief Timer比较类，用于set比较
	 */
	struct Compare{
		bool operator()(Timer::ptr lhs, Timer::ptr rhs){
			if(!lhs && !rhs){
				return false;
			}
			if(!lhs){
				return true;
			}
			if(!rhs){
				return false;
			}
			if(lhs->m_next < rhs->m_next){
				return true; 
			}
			if(lhs->m_next > rhs->m_next){
				return false;
			}
			return lhs.get() < rhs.get();
		}
	};

private:
	//定时器间隔
	uint64_t m_ms = 0; 
	//回调函数
	std::function<void()> m_cb = nullptr;
	//是否循环
	bool m_loop = false; 
	//定时器下一个触发时间
	uint64_t m_next = 0; 
	//定时器开始时间
	uint64_t m_first = 0 ;
	//定时器管理对象指针
	TimerMgr * m_mgr;
};

/**
 * @brief 定时器管理类
 */
class TimerMgr{
friend class Timer;
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<TimerMgr> ptr;
	/**
	 * @brief 定义锁类型
	 */
	typedef SpinLock MutexType;
public:
	/**
	 * @brief 构造函数
	 */
	TimerMgr();

	/**
	 * @brief 虚析构函数
	 */
	virtual ~TimerMgr();

	/**
	 * @brief 添加定时器
	 * @param[in] ms 定时器时间间隔
	 * @param[in] cb 回调函数
	 * @param[in] isloop 是否循环
	 */
	Timer::ptr addTimer(uint64_t ms , std::function<void()> cb , bool isloop = false);
	
	/**
	 * @brief 添加定时器
	 * @param[in] ms 定时器时间间隔
	 * @param[in] cb 回调函数
	 * @param[in] weak 定时器执行条件，当weak.lock不为空才执行
	 * @param[in] isloop 是否循环
	 */	
	Timer::ptr addCondTimer(uint64_t ms , std::function<void()> cb ,std::weak_ptr<void> weak, bool isloop = false);

	/**
	 * @brief 删除定时器
	 * @param[in] timer 定时器
	 */
	void delTimer(Timer::ptr timer);

	/**
	 * @brief 清除所有定时器，并将超时的定时器取出来
	 * @param[out] vec 存放超时定时器
	 */
	void clearTimerToVec(std::vector<Timer::ptr>& vec);
	
	/**
	 * @brief 获得超时定时器
	 * @param[out] vec 存放超时定时器
	 */
	void getTimeOutTimer(std::vector<Timer::ptr> & vec);
	
	/**
	 * @brief 有定时器是所有定时器最快触发的，通知一下线程 
	 */
	virtual void timerTickle();
	
	/**
	 * @brief 获得最快的那个定时器还有多久触发
	 */
	uint64_t getFirstTimerTimeOut();
	
	/**
	 * @brief 添加定时器
	 * @param[in] timer 定时器
	 */
	void addTimer(Timer::ptr timer);
private:
	// set ,存储所有的定时器
	std::set<Timer::ptr,Timer::Compare> m_timers;
	//锁
	MutexType m_mutex;
};

}

#endif 
