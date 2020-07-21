#ifndef __TADPOLE_FIGHTLANDLORD_ROOM_H__
#define __TADPOLE_FIGHTLANDLORD_ROOM_H__

#include <memory>
#include <map>
#include "src/socket.h"
#include "src/singleton.h"
#include "src/mutex.h"
#include <unordered_set>

namespace tadpole{

class Desktop{
friend class PlayRoom;
public:
	typedef std::shared_ptr<Desktop> ptr;
	/**
	 * @brief 添加玩家
	 */

	Desktop();

	int addUser(Socket::ptr sock,const std::string & name);
	/**
	 * @brief 删除玩家
	 */
	void delUser(Socket::ptr sock);

	/** 
	 * @brief 通知其他玩家
	 */
	void tickleOtherUser(Socket::ptr sock,std::function<void(Socket::ptr)> cb);
	/**
	 * @brief 获得卡牌
	 */
	std::shared_ptr<uint8_t> getCards(){return m_cards;}

	/**
	 * @brief 开始游戏
	 */
	static int start(Desktop::ptr desktop);

	/**
	 * @brief 开始出牌
	 */
	static int startPutCard(Desktop::ptr desktop,Socket::ptr sock,int pri);
	
	int addLeft();
private:
	//人数
	int m_userCount = 0;
	//地主是谁
	int m_landlord = -1;
	//退出人数
	int m_outCount = 0 ; 
	//谁出牌
	int m_who = 0 ; 
	//桌子
	std::array<Socket::ptr,3> m_desktop;
	//保存叫地主时的状态
	std::array<int,3> m_callStatus; 
	//桌子对应人的名称
	std::array<std::string,3> m_players;
	//每个人开始出牌的时间
	std::array<uint64_t,3> m_begintime;
	//这桌子上的牌
	std::shared_ptr<uint8_t> m_cards;
	//还剩余多少
	std::array<std::unordered_set<uint8_t>,3> m_left;
};

class PlayRoom{
friend class Desktop;
public:
	typedef std::shared_ptr<PlayRoom> ptr;
	typedef SpinLock MutexType;
	/**
	 * @brief构造函数
	 */
	PlayRoom();
	
	/**
	 * @brief 接收用户加入到房间
	 */
	std::pair<int,uint8_t> accessToRoom(Socket::ptr,const std::string& name);

	/**
	 * @brief 让用户退出房间，如果已经开始，就无法退出
	 */
	void outFromRoom(Socket::ptr);

	/**
	 * @brief 玩的时候退出房间
	 */
	void playingOutFromRoom(Socket::ptr);

	/**
	 * @brief 获得房间
	 * @param[in] id 房间id
	 */
	Desktop::ptr getRoom(uint32_t id);

private:
	//房间id
	int m_desktopId = 1;
	//人未满的房间
	Desktop::ptr m_accessRoom;
	//人已满的房间
	std::map<int,Desktop::ptr> m_Rooms; 
	//锁
	MutexType m_mutex; 
};

typedef Singleton<PlayRoom> RoomMgr;

}
#endif 
