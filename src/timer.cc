#include "timer.h"
#include "util.h"
#include "log.h"

namespace tadpole{

Timer::Timer(uint64_t ms,std::function<void()> cb,TimerMgr * mgr,bool isloop)
	:m_ms(ms)
	,m_cb(cb)
	,m_loop(isloop)
	,m_mgr(mgr){
	m_first = GetTimeOfMS();
	m_next = m_first + m_ms; 
}

Timer::Timer(uint64_t next)
	:m_next(next){

}

void Timer::cancel(){
	if(m_mgr){
		m_mgr->delTimer(this->shared_from_this());
	}
}

void Timer::reset(){
	if(m_mgr){
		Timer::ptr timer = this->shared_from_this();
		m_mgr->delTimer(timer);
		m_mgr->addTimer(timer->m_ms,timer->m_cb,timer->m_loop);
	}
}

void Timer::reflush(){
	if(m_mgr){
		Timer::ptr timer = this->shared_from_this();
		m_mgr->delTimer(timer);
		m_next = GetTimeOfMS();
		m_mgr->addTimer(timer); 
	}
}

bool Timer::checkTime(){
	if(GetTimeOfMS() < m_first || GetTimeOfMS() >=(m_next + 5*1000)){
		return false;
	}	
	return true;
}

TimerMgr::TimerMgr(){}

TimerMgr::~TimerMgr(){}

uint64_t TimerMgr::getFirstTimerTimeOut(){
	MutexType::Lock lock(m_mutex);
	if(m_timers.empty()){
		return ~0ull;
	}
	auto it = m_timers.begin();
	lock.unlock();
	if((*it)->m_next < GetTimeOfMS())
		return 0 ; 
	else 
		return (*it)->m_next - GetTimeOfMS();
}

Timer::ptr TimerMgr::addTimer(uint64_t ms , std::function<void()> cb , bool isloop){
	Timer::ptr timer(new Timer(ms,cb,this,isloop));
	MutexType::Lock lock(m_mutex);
	auto begin = m_timers.insert(timer).first;
	if(begin == m_timers.begin()){
		timerTickle();
	}
	return timer;
}

void TimerMgr::addTimer(Timer::ptr timer){
	MutexType::Lock lock(m_mutex);
	auto begin = m_timers.insert(timer).first;
	if(begin == m_timers.begin()){
		timerTickle();
	}
}

static void OnTimer(std::weak_ptr<void> weak , std::function<void()> cb){
	if(weak.lock()){
		cb();
	}
}

Timer::ptr TimerMgr::addCondTimer(uint64_t ms , std::function<void()> cb ,std::weak_ptr<void> weak, bool isloop){
	return addTimer(ms,std::bind(&OnTimer,weak,cb) ,isloop);
}
void TimerMgr::delTimer(Timer::ptr timer){
	MutexType::Lock lock(m_mutex);
	if(m_timers.empty()){
		return ; 
	}
	auto it = m_timers.find(timer);
	if(it == m_timers.end()){
		return ; 
	}
	if(it == m_timers.begin()){
		timerTickle();
	}
	m_timers.erase(it);
}

void TimerMgr::getTimeOutTimer(std::vector<Timer::ptr> & vec){
	std::vector<Timer::ptr> loopTimer;
	uint64_t cur_time = GetTimeOfMS();
	Timer::ptr timer(new Timer(cur_time));
	MutexType::Lock lock(m_mutex);
	if(m_timers.empty()){
		return ; 
	}	
	auto it = m_timers.upper_bound(timer);
	if(it == m_timers.begin()){
		return ;
	}
	for(;it != m_timers.end();){
		if((*it)->m_next == cur_time){
			++it;
		}else{
			break;
		}
	}

	for(auto iter = m_timers.begin(); iter != it ; ++iter){
		if((*iter)->checkTime()){
			vec.push_back(*iter);
		}
		m_timers.erase(iter);
		if((*iter)->m_loop){
			loopTimer.push_back(*iter);
		}
	}
	for(auto &iter : loopTimer){
		iter->m_first = GetTimeOfMS();
		iter->m_next = iter->m_first + iter->m_ms;
		m_timers.insert(iter);
	}
}

void TimerMgr::clearTimerToVec(std::vector<Timer::ptr> & vec){
	getTimeOutTimer(vec);
	MutexType::Lock lock(m_mutex);
	m_timers.clear();
}

void TimerMgr::timerTickle(){
	
}

}
