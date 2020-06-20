#include "http_session.h"
#include "http_servlet.h"
#include "src/bytearray.h"
#include "src/config.h"
#include "src/log.h"

namespace tadpole{

Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

ConfigVar<uint32_t>::ptr g_http_header_max_size_conf =
				Config::Lookup<uint32_t>("http.header.max.size",4096,"http header buffs max size ");

ConfigVar<uint64_t>::ptr g_http_keep_alive_timeout_conf =
				Config::Lookup<uint64_t>("http.keep_alive.timeout",1000*60*2,"http keep alive timeout");

static uint32_t g_http_header_max_size = 0 ; 

static uint64_t g_http_keep_alive_timeout = 0 ; 

struct _sizeIniter{
	_sizeIniter(){
		g_http_header_max_size_conf->addListener([](uint32_t oldval , uint32_t newval){
			g_http_header_max_size = newval;
		});
		g_http_header_max_size = g_http_header_max_size_conf->getValue();

		g_http_keep_alive_timeout_conf->addListener([](uint32_t oldval , uint32_t newval){
			g_http_keep_alive_timeout = newval;
		});
		g_http_keep_alive_timeout = g_http_keep_alive_timeout_conf->getValue();
	}
};

static _sizeIniter sizeIniter;

HttpSession::HttpSession(Socket::ptr sock)
	:m_sock(sock){
	m_responce.reset(new HttpResponce);
}

HttpSession::~HttpSession(){
	m_sock->close();
}

void HttpSession::recvRequest(){
		//设置连接没有响应的时间
		m_sock->setRecvTimeout(g_http_keep_alive_timeout);
		//创建一个buffs,大小为4096
		std::shared_ptr<char> buffs(new char[g_http_header_max_size],[](const char * buf){
				delete[] buf;
			});
		//接收一个buf
		int ret = m_sock->recv(buffs.get(),g_http_header_max_size - 1);
		if(ret <= 0){
			m_isTimeout = true;
			m_responce->setClose(true);
			return ; 
		}
		//创建一个解析类
		HttpRequestParser::ptr preq(new HttpRequestParser);
		//开始解析
		preq->execute(buffs.get(),strlen(buffs.get()));
		
		if(!preq->isFinish()){
			TADPOLE_LOG_WARN(g_logger)<<"header is long !";
			return; 	
		}
		auto quest = preq->getRequest();
	
		std::string con = quest->getField("Connection");
		if(strncasecmp(&con[0],"keep-alive",sizeof("keep-alive")) == 0){
			m_responce->setClose(false);
		}	
		
		m_responce->setField("Server","tadpole/1.0.0");
		m_responce->setField("Connection",con);
		m_responce->setField("Content-type","text/html;charset=utf-8");
	//	TADPOLE_LOG_INFO(g_logger) << quest->toString();
		ServletMgr::GetInstance()->getServlet(quest->getPath())->handle(m_responce);
}

void HttpSession::sendResponce(){
		if(m_isTimeout){
			return ; 
		}
		std::stringstream ss; 
		ss << m_responce->getBody().size();
		m_responce->setField("Content-Length",ss.str());

		std::string str = m_responce->toString();
		m_sock->send(&str[0],str.size());
	//	TADPOLE_LOG_INFO(g_logger)<<m_responce->toString();
}

}
