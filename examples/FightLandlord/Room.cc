#include "Room.h"
#include "src/macro.h"
#include "src/iomanager.h"
#include "protocal.h"
#include "src/log.h"
#include "src/fdmanager.h"

#include <time.h>
#include "src/util.h"

namespace tadpole{

Desktop::Desktop(){
	srand(time(NULL));
	for(int i = 0 ; i < 3; ++i){
		m_desktop[i] = nullptr;
		m_callStatus[i] = 1;
	}
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

//随机做座位,返回出牌顺序：
int Desktop::addUser(Socket::ptr sock,const std::string &name ){
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
						,std::placeholders::_1,name,i));
	return i; 
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
			it = nullptr;
			--m_userCount;
			return ; 
		}
	}
}

PlayRoom::PlayRoom(){
	m_accessRoom.reset(new Desktop);
}

//将牌转到hash容器里面，好查找
int Desktop::addLeft(){
	for(int i = 0 ; i < 3; ++i){
		for(int j = 0 ; j < 17 ; ++j){
			m_left[i].insert(m_cards.get()[j+(i*17)]);
		}
	}
	if(m_landlord == -1){
		return -1; 
	}
	for(int i = 0 ; i < 3 ; ++i){
		m_left[m_landlord].insert(m_cards.get()[51+i]);
	}
	return 0 ; 
}

int recvOther(Socket::ptr sock){
	ByteArray::ptr ba(new ByteArray(1024));
	sock->setRecvTimeout(30*1000);
	sock->recvTo(ba);
	int16_t type = 0 ; 
	ba->readFInt16(type);
	//根据情况返回
	return 0 ; 
}

int Desktop::startPutCard(Desktop::ptr desktop,Socket::ptr sock,int pri){
	desktop->m_who = desktop->m_landlord; 
	IOManager* iom = IOManager::GetCurIOM();
	ByteArray::ptr ba(new ByteArray);
	bool once = true;
	while(1){
		//只在第一次进来的时候执行
		if(once){
			//通知一个人出牌
			iom->schedule(std::bind(Asking::send,sock,desktop->m_who%3));
			once = false; 
		}
		//设置每个人的超时时间
		int ret = 0 ;
		//若为出牌人给30考虑时间,否则给120秒
		if(desktop->m_who == pri){
			sock->setRecvTimeout(30*1000);
		}else{
			sock->setRecvTimeout(2*60*1000);
		}

		//是否胜利
		bool isWin = false; 
		
		//如果不是出牌的人，就接收聊天语句
		while(1){
			//接收玩家请求
			ret = sock->recvTo(ba);
			if(ret <= 0){
				TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("root"))<<"ret:"<<ret ; 
				//暂时直接返回
				if(ret == -2){
						//超时直接不出
						for(auto & it : desktop->m_desktop){
							//展示所有的牌
							iom->schedule(std::bind(ShowCard::send,it,nullptr,0,desktop->m_who,0,0));	
							//通知一个人出牌
							iom->schedule(std::bind(Asking::send,it,(desktop->m_who+1)%3));
						}

						//以下是设置下一个出牌人
						//设置下一个人出牌的超时时间
						desktop->m_desktop[(desktop->m_who + 1)%3]->setRecvTimeout(30*1000);
						//让它重新recv
						iom->cancelEvent(desktop->m_desktop[(desktop->m_who + 1)%3]->getSocket(),IOManager::Event::READ);
						//设置这个玩家的开始出牌时间
						desktop->m_begintime[(desktop->m_who + 1)%3]= GetTimeOfMS();
						break; 
				}
				return 0; 
			}
			
			int16_t type = 0 ; 
			ba->readFInt16(type);
			if(type ==  (uint16_t)MsgReq::PUTCARD){
				//接收牌，并通知出了什么牌
				isWin = false;  
				PutCard::parser(ba,[desktop,iom,&isWin](std::shared_ptr<char> buf
											 ,uint8_t count,double cardType,int cardNum){
					uint8_t *card = (uint8_t*)buf.get();
				 	for(uint8_t j = 0 ; j < count ; ++j){
						desktop->m_left[desktop->m_who%3].erase(card[j]);
					}
					if(!desktop->m_left[desktop->m_who%3].empty()){
						for(auto & it : desktop->m_desktop){
							//展示所有的牌
							iom->schedule(std::bind(ShowCard::send,it,buf,count,desktop->m_who%3,cardType,cardNum));	
							//通知一个人出牌
							iom->schedule(std::bind(Asking::send,it,(desktop->m_who+1)%3));
						}

						//以下是设置下一个出牌人
						//设置下一个人出牌的超时时间
						desktop->m_desktop[(desktop->m_who + 1)%3]->setRecvTimeout(30*1000);
						//让它重新recv
						iom->cancelEvent(desktop->m_desktop[(desktop->m_who + 1)%3]->getSocket(),IOManager::Event::READ);
						//设置这个玩家的开始出牌时间
						desktop->m_begintime[(desktop->m_who + 1)%3]= GetTimeOfMS();
					}else {
						//win
						isWin = true; 
					}
				});
				break;
			}else{ 
				//执行任务其它任务
				//一般都为iom->schedule
			}
			//如果是自己出牌再发其它消息，就重置一下时间
			if(desktop->m_who == pri){
				//重新设置一下recv timeout 
				uint64_t timeout = (30*1000) - (GetTimeOfMS() - desktop->m_begintime[pri]);
				timeout = (timeout < 0) ? 0 : timeout;
				desktop->m_desktop[desktop->m_who%3]->setRecvTimeout(timeout);
			}
		}
		if(isWin){
			break;
			//并通知所有客户端
		}
		if(desktop->m_outCount == 3){
			//所有用户都退出了
			break;
		}
		++desktop->m_who;
	}
	return 0 ; 
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
int Desktop::start(Desktop::ptr desktop){
	ByteArray::ptr ba(new ByteArray);
	IOManager* iom = IOManager::GetCurIOM();
	//删除所有可读事件

	for(auto & it : desktop->m_desktop){
		it->setRecvTimeout(8*1000);
	}
	for(int i = 0 ; i < 4; ++i){
		//if(desktop->m_callStatus[i%3] == 0){
			//0表示已经不抢了
		//	continue; 
		//}
		//-1表示没人叫地主
		if(i == 3){
			if(desktop->m_landlord == -1){
				break; 
			}else if(desktop->m_landlord == 0){//表示自己抢了地主，没人和你抢
				break; 
			}else{
				//表示最后一个抢地主的人是谁
				while(desktop->m_callStatus[i%3] == 0){
					++i;
				}
				//如果这个人是地主，就直接给他
				if(desktop->m_landlord == i%3){
					break;
				}
			}
		}
		int callorrob = (desktop->m_landlord == -1) ? 1 : 2 ;
		for(auto &it : desktop->m_desktop){
			//通知客户端谁抢地主
			iom->schedule(std::bind(&CallLandlord::send,it,i%3,callorrob));
		}
		uint8_t iscall = 0 ; 
		//给抢地主的人8秒考虑
		desktop->m_desktop[i%3]->setRecvTimeout(8*1000);
		uint64_t timebegin = GetTimeOfMS();
		
		int ret = 0 ; 
		//若发其它信息过来。处理后继续倒计时
		while(1){
			//取消掉原来其他人的读事件
			for(int j= 0 ; j < 3; ++j){
				tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(desktop->m_desktop[j]->getSocket());
				sts->setAutoStop(true);
				iom->cancelEvent(desktop->m_desktop[j]->getSocket(),tadpole::IOManager::Event::READ);
			}	
			//接收其它玩家请求
			for(int j = 1 ; j <= 2; ++j){
				iom->schedule(std::bind(&recvOther
								,desktop->m_desktop[(i+j)%3]));
			}
	
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
			uint64_t timeout = (8*1000) - (GetTimeOfMS() - timebegin);
			timeout = (timeout < 0) ? 0 : timeout;
			desktop->m_desktop[i%3]->setRecvTimeout(timeout);
		}

		if(ret <= 0){
			//直接托管
			iscall = 0 ; 
		}
		//标记某个玩家抢没抢地主
		desktop->m_callStatus[i%3] = iscall;
		//不叫地主不变，叫了地主就变了
		desktop->m_landlord = (iscall == 0) ? desktop->m_landlord : i%3; 
		//TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("root"))<< desktop->m_landlord;
		//接收并通知
		for(auto &it : desktop->m_desktop){
			//通知客户端谁抢了地主
			iom->schedule(std::bind(&TickleIsCall::send,it,i%3,iscall));
		}
	}
	
	if(desktop->m_landlord == -1){
		//重新开始
		return 0; 
	}else{
		//给给这个人地主牌
		for(auto &it : desktop->m_desktop){	
			iom->schedule(std::bind(&LandlordCard::send,it,desktop->m_cards.get() + 51,desktop->m_landlord));
		}
	}
	//将牌换到hash,好计算
	desktop->addLeft();
	
	int i = 0 ; 
	for(auto & it : desktop->m_desktop){
		//先把原先的事件取消,没有事件也就不会取消	
		tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(it->getSocket());
		sts->setAutoStop(true);
		iom->cancelEvent(it->getSocket(),IOManager::Event::READ);
		iom->schedule(std::bind(Desktop::startPutCard,desktop,it,i));
		++i;
	}
	//开始出牌

	return 0; 
}

//加入牌桌
std::pair<int,uint8_t> PlayRoom::accessToRoom(Socket::ptr sock,const std::string & name){
	int roomId = m_desktopId;
	bool isFull = false;
	MutexType::Lock lock(m_mutex);
	//添加一个用户
	uint8_t pri = m_accessRoom->addUser(sock,name);
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
		isFull = true;	
	}
	//TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root"))<< (int)pri;
	std::unordered_map<uint8_t,std::string> names; 
	for(int i = 0 ; i < 3 ; ++i){
		if(m_accessRoom->m_desktop[i]){
			if(m_accessRoom->m_desktop[i]->getSocket() != sock->getSocket()){
				names.insert(std::make_pair((uint8_t)i,
								m_accessRoom->m_players[i]));
				TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root")) << "str: " <<m_accessRoom->m_players[i];
			}
		}
	}

	IOManager* iom = IOManager::GetCurIOM();
	iom->schedule(std::bind(&DisplayPearName::send,sock,names));
	
	//0.1秒后开始抢地主
	if(isFull){
		iom->addTimer(5000,std::bind(&Desktop::start,m_accessRoom),false);

		for(auto & it : m_accessRoom->m_desktop){
			//先把原先的事件取消,没有事件也就不会取消
			
			tadpole::FdSts::ptr sts= tadpole::FdMgr::GetInstance()->get(it->getSocket());
			sts->setAutoStop(true);
			iom->cancelEvent(it->getSocket(),IOManager::Event::READ);
		}
		m_accessRoom.reset(new Desktop);
	}
	return std::make_pair(roomId,pri);	
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
