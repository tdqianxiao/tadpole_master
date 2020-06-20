#include "Room.h"
#include "src/macro.h"
#include "src/iomanager.h"
#include "protocal.h"

#include <time.h>
#include "src/util.h"

namespace tadpole{

Desktop::Desktop(){
	srand(time(NULL));
}

/**
 * @brief 遍历这个桌子，如果有人就通知一些，
 */
void Desktop::tickleOtherUser(Socket::ptr sock
							 ,std::function<void(Socket::ptr)> cb){
	for(int i = 0 ; i < 3; ++i){
		if(m_desktop[i] != nullptr){
			if(sock->getSocket() != m_desktop[i]->getSocket()){
				IOManager* iom = IOManager::GetCurIOM();
				iom->schedule(std::bind(cb,m_desktop[i]));
			}
		}
	}	
}

//随机做座位
void Desktop::addUser(Socket::ptr sock,const std::string &name ){
	//玩家数量一定不等于3
	TADPOLE_ASSERT(m_userCount != 3);
	int i = rand()%3; 
	if(m_userCount == 1){
		while(m_desktop[i] != nullptr){
			i = (i+1)%3;
		}	
	}else if(m_userCount == 2){
		for(int n = 0 ; n < 3; ++n){
			//桌子上哪个位置为空
			if(m_desktop[n] == nullptr){
				i = n ; 
				break; 
			}
		}	
	}
	m_desktop[i] = sock; 
	++m_userCount ; 
	m_players[i] = name;
	//通知其他用户
	tickleOtherUser(sock,std::bind(TickleAddUser::send
						,std::placeholders::_1,name));
}

void Desktop::delUser(Socket::ptr sock){
	if(m_userCount == 0){
		return ; 
	}
	for(auto & it : m_desktop){
		if(it == nullptr){
			continue ; 
		}
		if(it->getSocket() == sock->getSocket()){
			it == nullptr;
			--m_userCount;
			return ; 
		}
	}
}

PlayRoom::PlayRoom(){
	m_accessRoom.reset(new Desktop);
}

/**
 * @brief 大致流程，接管这一张牌桌
 * ，并删除掉原来文件描述符上所有的可读事件
 * 抢地主流程： 
 * 服务器发送通知告诉客户端谁该抢地主
 * 该抢地主的客户端有30秒回应，不回应等于掉线，直接pass
 * 然后通知3个客户端该客户端是否抢了地主，
 * 继续下一轮抢地主
 * 共4轮
 */
int PlayRoom::start(Desktop::ptr desktop){
	ByteArray::ptr ba(new ByteArray);
	IOManager* iom = IOManager::GetCurIOM();
	//删除所有可读事件
	for(auto & it : desktop->m_desktop){
		iom->delEvent(it->getSocket(),IOManager::Event::READ);
	}
	for(int i = 0 ; i < 4; ++i){
		for(auto &it : desktop->m_desktop){
			//通知客户端谁抢地主
			iom->schedule(std::bind(&CallLandlord::send,it,i%3,(i==0)?1:2));
		}
		uint8_t iscall = 0 ; 
		//给抢地主的人30秒考虑
		desktop->m_desktop[i%3]->setRecvTimeout(30*1000);
		uint64_t timebegin = GetTimeOfMS();
		
		int ret = 0 ; 
		//若发其它信息过来。处理后继续倒计时
		while(1){
			ret = desktop->m_desktop[i%3]->recvTo(ba);
			if(ret <= 0)break;
			
			int16_t type = 0 ; 
			ba->readFInt16(type);
			if(type ==  (uint16_t)MsgReq::ISCALL){
				//如果是真确的叫地主请求就退出循环
				//解析客户端抢地主请求，并给iscall 赋值
				IsCallLand::parser(ba,iscall);
				break;
			}else {
				//执行任务其它任务
				//一般都为iom->schedule
			}
			//重新设置一下recv timeout 
			uint64_t timeout = (30*1000) - (GetTimeOfMS() - timebegin);
			timeout = (timeout < 0) ? 0 : timeout;
			desktop->m_desktop[i%3]->setRecvTimeout(timeout);
		}

		if(ret <= 0){
			//直接托管
			iscall = 0 ; 
		}
		//接收并通知
		for(auto &it : desktop->m_desktop){
			//通知客户端谁抢了地主
			iom->schedule(std::bind(&TickleIsCall::send,it,i%3,iscall));
		}
	}

	return 0; 
}

//加入牌桌
int PlayRoom::accessToRoom(Socket::ptr sock,const std::string & name){
	int roomId = m_desktopId;
	MutexType::Lock lock(m_mutex);
	//添加一个用户
	m_accessRoom->addUser(sock,name);
	if(m_accessRoom->m_userCount == 3){
		if(m_desktopId == 3000000){
			m_desktopId = 1;
		}
		m_accessRoom->m_cards = Random::GetCard54();
		m_Rooms.insert(std::make_pair(m_desktopId++
							,m_accessRoom));
		IOManager* iom = IOManager::GetCurIOM();
		for(int i = 0 ; i < 3; ++i){
			//暂定为谁先加入谁先出牌
			iom->schedule(std::bind(&CardDistributor::send
						,m_accessRoom->m_desktop[i]
						,m_accessRoom->m_cards.get() + (i * 17)
						,i));
		}
		//一个牌桌的开始
		iom->schedule(std::bind(&PlayRoom::start,m_accessRoom));
		m_accessRoom.reset(new Desktop);
	}
	return roomId;	
}

//退出牌桌
void PlayRoom::outFromRoom(Socket::ptr sock){
	MutexType::Lock lock(m_mutex);
	m_accessRoom->delUser(sock);
}

//获得房间，也相当与牌桌
Desktop::ptr PlayRoom::getRoom(uint32_t id){
	MutexType::Lock lock(m_mutex);
	auto it = m_Rooms.find(id);
	if(it == m_Rooms.end()){
		return nullptr;
	}
	return it->second;
}

}
