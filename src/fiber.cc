#include "src/fiber.h"
#include "src/log.h"
#include "src/config.h"
#include "src/macro.h"
#include <atomic>

namespace tadpole{

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
		Config::Lookup<uint32_t>("fiber.stack_size",1024*128,"fiber stack size");

static thread_local Fiber * t_curFiber ; 
static thread_local Fiber::ptr t_mainFiber ; 
static thread_local Fiber::ptr t_firstFiber ; 

static std::atomic<uint32_t> g_fiber_count = {0} ; 
static std::atomic<uint32_t> g_fiber_id = {0} ;

uint32_t Fiber::GetFiberId(){
	if(t_curFiber){
		return t_curFiber->getFiberId();
	}
	return 0 ; 
}

void Fiber::SetMainFiber(Fiber::ptr f){
	t_mainFiber = f;
}

void Fiber::SetCurFiber(Fiber *f){
	t_curFiber = f; 
}

Fiber::ptr Fiber::GetMainFiber(){
	return t_mainFiber;
}

class MemoryAlloc{
public:
	static void *Alloc(size_t size){
		return malloc(size);
	}

	static void Dealloc(void * p , size_t size){
		free(p);
	}
};

using StackPool = MemoryAlloc;

//static std::atomic<int> myCount = {1};
//static std::atomic<int> xCount = {1};

Fiber::Fiber(){
	m_state = EXEC;
	SetCurFiber(this);
	if(-1 == getcontext(&m_ctx)){
		TADPOLE_LOG_ERROR(g_logger)<< "getcontext non success !";
		throw std::logic_error("getcontext non success!");
	}	
//	TADPOLE_LOG_ERROR(g_logger)<< "create" <<myCount;
//	++myCount;
	++g_fiber_count;
}

Fiber::Fiber(std::function<void()> cb , const uint32_t & stacksize)
	:m_id(++g_fiber_id)
	,m_state(INIT)
	,m_cb(cb){
	if(-1 == getcontext(&m_ctx)){
		TADPOLE_LOG_ERROR(g_logger)<< "getcontext non success !";
		throw std::logic_error("getcontext non success!");
	}
	++g_fiber_count;
	m_stackSize = stacksize ? stacksize : g_fiber_stack_size->getValue();
	
	m_stack = StackPool::Alloc(m_stackSize);

	m_ctx.uc_link = nullptr;
	m_ctx.uc_stack.ss_sp = m_stack; 
	m_ctx.uc_stack.ss_size = m_stackSize;

//	TADPOLE_LOG_ERROR(g_logger)<< "create" <<myCount;
//	++myCount;
	makecontext(&m_ctx,&Fiber::MainFunc,0);
}

Fiber::~Fiber(){
//	TADPOLE_LOG_ERROR(g_logger)<< "~ " <<xCount;
//	++xCount;
	if(this == t_mainFiber.get()){
		t_mainFiber = nullptr;	
	}else{
		StackPool::Dealloc(m_stack,m_stackSize);	
	}
}

Fiber::ptr Fiber::GetCurFiber(){
	if(t_curFiber){
		return t_curFiber->shared_from_this(); 
	}
	Fiber::ptr main_fiber(new Fiber());
	TADPOLE_ASSERT(t_curFiber);
	t_mainFiber = main_fiber; 
	return main_fiber;
}

void Fiber::swapIn(){
	TADPOLE_ASSERT(m_state != EXEC);
	m_state = EXEC;
	Fiber* main_fiber = t_mainFiber.get();
	main_fiber->m_state = READY;
	SetCurFiber(this);
	if(-1 == swapcontext(&main_fiber->m_ctx,&m_ctx)){	
		TADPOLE_LOG_ERROR(g_logger)<< "swapcontext non success !";
		throw std::logic_error("swapcontext non success!");
	}
}

void Fiber::swapOut(){
	TADPOLE_ASSERT(m_state != EXEC);
	Fiber* main_fiber = t_mainFiber.get();
	main_fiber->m_state = EXEC;
	SetCurFiber(main_fiber);
	if(-1 == swapcontext(&m_ctx,&main_fiber->m_ctx)){	
		TADPOLE_LOG_ERROR(g_logger)<< "swapcontext non success !";
		throw std::logic_error("swapcontext non success!");
	}
}

void Fiber::call(){
	TADPOLE_ASSERT(m_state != EXEC);
	m_state = EXEC;
	SetCurFiber(this);
	Fiber* main_fiber = t_mainFiber.get();
	t_firstFiber = t_mainFiber;
	t_mainFiber = this->shared_from_this();
	t_firstFiber->m_state = HOLD;
	if(-1 == swapcontext(&main_fiber->m_ctx,&m_ctx)){	
		TADPOLE_LOG_ERROR(g_logger)<< "swapcontext non success !";
		throw std::logic_error("swapcontext non success!");
	}
}

void Fiber::back(){
	TADPOLE_ASSERT(t_firstFiber && t_firstFiber->m_state == HOLD);
	t_firstFiber->m_state = EXEC;
	Fiber * old_fiber = t_curFiber;
	SetCurFiber(t_firstFiber.get());
	SetMainFiber(t_firstFiber);
	t_firstFiber.reset();
	if(-1 == swapcontext(&old_fiber->m_ctx,&t_mainFiber->m_ctx)){	
		TADPOLE_LOG_ERROR(g_logger)<< "swapcontext non success !";
		throw std::logic_error("swapcontext non success!");
	}
}

void Fiber::YieldToReady(){
	TADPOLE_ASSERT(t_curFiber);
	t_curFiber->m_state = READY;
	t_curFiber->swapOut();
}

void Fiber::YieldToHold(){
	TADPOLE_ASSERT(t_curFiber);
	t_curFiber->m_state = HOLD;
	t_curFiber->swapOut();
}

void Fiber::MainFunc(){
	try{
		if(t_curFiber->m_cb){
			t_curFiber->m_cb();
		}
		t_curFiber->m_state = TERM; 
	}catch(std::exception & e){
		t_curFiber->m_state = EXCEPT;
		TADPOLE_LOG_ERROR(g_logger) << "fiber exception : "
									<< e.what();
	}catch(...){
		TADPOLE_LOG_ERROR(g_logger) << "fiber exception";
		t_curFiber->m_state = EXCEPT;
	}
	if(t_curFiber == t_mainFiber.get()){
		back();
	}else{
		t_curFiber->swapOut();
	}
}
}
