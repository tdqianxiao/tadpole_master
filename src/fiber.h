#ifndef __TADPOLE_FIBER_H__
#define __TADPOLE_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

namespace tadpole{

/**
 * @brief 协程类
 */
class Fiber: public std::enable_shared_from_this<Fiber>{
public:
	/**
	 * @brief 定义协程智能指针
	 */
	typedef std::shared_ptr<Fiber> ptr;
public:
	/**
	 * @brief 协程状态
	 */
	enum State{
		INIT = 0,
		READY,
		HOLD,
		EXEC,
		TERM,
		EXCEPT
	};
public:	
	/**
	 * @brief 构造函数
	 * @param[in] cb 回调函数
	 * @param[in] stacksize 协程栈大小
	 */
	Fiber(std::function<void()> cb,const uint32_t & stacksize = 0);

	/**
	 * @brief 析构函数
	 */
	~Fiber();

	/**
	 * @brief 设置协程状态
	 */
	void setState(State s){m_state = s;}

	/**
	 * @brief 获得协程状态
	 */
 
 	State getState()const {return m_state;}
	/**
	 * @brief 获得协程id
	 */
	uint32_t getFiberId(){return m_id;}
	
	/**
	 * @brief 从主协程切换到子协程，并可以通过swapout返回
	 */
	void swapIn();

	/**
	 * @brief 从当前协程切换到主协程
	 */
	void swapOut();

	/**
	 * @brief 把主线程转让给子线程
	 */
	void call();

	/**
	 * @brief 回到原来的主线程
	 */
	static void back();

	/**
	 * @brief 获得当前协程id
	 */
	static uint32_t GetFiberId();
	
	/**
	 * @brief 从子协程切换到主协程，并置子协程为READY状态
	 */
	static void YieldToReady();
	
	/**
	 * @brief 从子协程切换到主协程，并置子协程为HOLD状态
	 */
	static void YieldToHold();
	
	/**
	 * @brief 获得主协程
	 */
	static Fiber::ptr GetMainFiber();

	/**
	 * @brief 获得当前协程
	 */
	static Fiber::ptr GetCurFiber();

	/**
	 * @brief 设置主协程
	 * @param[in] f 协程智能指针
	 */
	static void SetMainFiber(Fiber::ptr f);

	/**
	 * @brief 设置当前协程
	 * @param[in] f 协程指针
	 */
	static void SetCurFiber(Fiber *f);
private:
	/**
	 * @brief 用于线程的主协程的创建
	 */
	Fiber();
	
	/**
	 * @brief 协程执行函数
	 */
	static void MainFunc();

private:
	//协程id
	uint32_t m_id = 0 ;
	//协程状态
	State m_state = INIT; 
	//协程栈指针
	void * m_stack = nullptr;
	//协程回调函数
	std::function<void()> m_cb;
	//协程栈大小
	uint32_t m_stackSize = 0 ; 
	//协程结构
	ucontext_t m_ctx; 
};

}

#endif 
