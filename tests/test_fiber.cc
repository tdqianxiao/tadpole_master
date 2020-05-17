#include "fiber.h"
#include "log.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void fun2(){
	TADPOLE_LOG_INFO(g_logger) << "fun2 begin";
	TADPOLE_LOG_INFO(g_logger) << "fun2 end";
}

void fun(){
	TADPOLE_LOG_INFO(g_logger) << "fun begin";
	
	tadpole::Fiber::ptr fiber2(new tadpole::Fiber(&fun2));
	fiber2->swapIn();
	TADPOLE_LOG_INFO(g_logger)<< "fun end";
}

void test_fiber(){
	TADPOLE_LOG_INFO(g_logger) << "test_fiber begin";
	tadpole::Fiber::GetCurFiber();
	tadpole::Fiber::ptr fiber1(new tadpole::Fiber(&fun));
	fiber1->call();
	TADPOLE_LOG_INFO(g_logger) << "test_fiber end";
}

int main (){
	test_fiber();
}
