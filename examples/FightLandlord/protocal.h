#ifndef __TADPOLE_FIGHT_LARDLORD_PROTOCAL_H__
#define __TADPOLE_FIGHT_LARDLORD_PROTOCAL_H__

#include "src/bytearray.h"
#include "src/socket.h"
#include "Room.h"
#include "Random.h"
#include <iostream>

//协议type
//1 加入到房间
//1001 通知客户端加入成功。并返回房间号
//2 出牌
//1002 出牌通知
//501 通知房间其他用户
//502 发牌

namespace tadpole{

enum class MsgReq{
	LOGIN = 1,
	ADDSESS = 2,
	ISCALL = 3
};

enum class MsgRsp{
	LOGINRSP = 501,//登录返回
	ADDSESSRSP = 502,//加入会议返回
	CARDDISC = 503,//卡牌分发
	ADDTICKLE = 504,//加入通知
	CALLLAND = 505, //叫地主
	TICKLEISCALL = 506 //通知是否叫地主
};

//参考协议头        
struct MessageHead {
	int16_t type;//协议类型
	uint16_t bodysize;//body大小
	int key;//校验字段
};

/**
 * @brief 登录协议的解析
 */
struct Login{
	static int parser(ByteArray::ptr ba , 
			std::function<void(const std::string & acc
							  ,const std::string & pwd)> cb);
};

/**
 * @brief 登录协议的响应
 */
struct LoginRsp{
	static int send(Socket::ptr sock,uint16_t status);
};

/**
 * @brief 加入房间协议解析
 */
struct AddSession{
	static int parser(ByteArray::ptr ba,std::function<int(
					const std::string& name)>);
};

/**
 * @brief 加入房间响应
 */
struct AddSessionRsp{
	static int send(Socket::ptr , int roomId);
};

/**
 * @brief 发牌协议
 * head : type 
 * 		  size 
 * 		  key 
 * body : priority 
 * 		  uint8[17]
 */
struct CardDistributor{
	static int send(Socket::ptr sock ,const uint8_t* cards,uint8_t priority);
};

/**
 * @brief 通知有人加入房间
 * head : type 
 * 		  size 
 * 		  key 
 * body : char * 
 */
struct TickleAddUser{
	static int send(Socket::ptr sock ,const std::string & name);
};

/**
 * @brief 是否叫地主
 */
struct CallLandlord{
	static int send(Socket::ptr sock ,uint8_t priority,uint8_t callOrRob);
};

struct IsCallLand{
	static int parser(ByteArray::ptr  ba , uint8_t & iscall);
};

struct TickleIsCall{
	static int send(Socket::ptr sock, uint8_t priority ,uint8_t callOrRob);
};

//应答器
class Responder{
public:
	typedef std::shared_ptr<Responder> ptr;

	Responder(Socket::ptr sock , ByteArray::ptr ba);
	int responce(int type);
private:
	Socket::ptr m_sock; 
	ByteArray::ptr m_ba; 
};

}

#endif 
