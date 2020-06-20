#ifndef __TADPOLE_HTTP_HTTP_SERVLET_H__
#define __TADPOLE_HTTP_HTTP_SERVLET_H__

#include <memory>
#include <functional>
#include <unordered_map>

#include "src/singleton.h"
#include "http_request.h"
#include "http_responce.h"

namespace tadpole{

/**
 * @brief servlet 类 
 */
class Servlet{
public:
	/**
	 * @brief 类型定义,智能指针
	 */
	typedef std::shared_ptr<Servlet> ptr;
	
	/**
	 * @brief 构造函数
	 */
	Servlet(){}

	/**
	 * @brief 虚析构函数
	 */
	virtual ~Servlet(){}

	/**
	 * @brief 统一接口，给responce设置body
	 * @param[out] res 应答类
	 */
	virtual void handle(HttpResponce::ptr res) = 0 ; 
};

/**
 * @brief 回调函数servlet类
 */
class FunctionServlet:public Servlet{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<FunctionServlet> ptr;

	/**
	 * @brief 回调类型函数定义
	 */
	typedef std::function<void(HttpResponce::ptr)> callback;

	/**
	 * @brief 构造函数
	 */
	FunctionServlet(callback cb);
	
	/**
	 * @brief 实现子类的handle,用callback设置body
	 */
	void handle(HttpResponce::ptr res)override; 
private:
	//回调函数
	callback m_cb ; 
};

/**
 * @brief 找不到资源servlet
 */
class NotFoundServlet:public Servlet{
public:
	/**
	 * @brief 类型定义,智能指针
	 */
	typedef std::shared_ptr<NotFoundServlet> ptr;

	/**
	 * @brief 重写子类的handle，setbody为not found 
	 */
	void handle(HttpResponce::ptr res)override;	
};

/**
 * @brief servlet 分发器
 */
class ServletDistributor{
public:
	/**
	 * @brief 类型定义,智能指针
	 */
	typedef std::shared_ptr<ServletDistributor> ptr;

	/**
	 * @brief 构造函数
	 * @param[in] def 默认的servlet
	 */
	ServletDistributor(Servlet::ptr def = nullptr);

	/**
	 * @brief 添加servlet
	 * @param[in] uri 资源路径
	 * @param[in] servlet uri对应的servlet
	 */
	void addServlet(const std::string & uri,
					Servlet::ptr servlet);

	/**
	 * @brief 删除servlet
	 * @param[in] uri 资源路径
	 */
	void delServlet(const std::string & uri);

	/**
	 * @brief 添加模糊匹配的servlet
	 * @param[in] uri 资源路径
	 * @param[in] servlet uri对应的servlet
	 */
	void addGlobServlet(const std::string & uri,
						Servlet::ptr servlet);

	/**
	 * @brief 删除模糊匹配的servlet
	 * @param[in] uri 资源路径
	 */
	void delGlobServlet(const std::string & uri);
	
	/**
	 * @brief 获得servlet
	 * @param[in] uri 资源路径
	 * @return servlet
	 */
	Servlet::ptr getServlet(const std::string & uri);
private:
	//存放精确的servlet
	std::unordered_map<std::string,Servlet::ptr> m_servlets;
	//存放模糊的servlet
	std::vector<std::pair<std::string,Servlet::ptr> > m_globServlets;
	//默认的servlet
	Servlet::ptr m_def;
};

//把servlet分发器做成单例类
typedef Singleton<ServletDistributor> ServletMgr;

/**
 * @brief html servlet
 */
class HtmlServlet:public Servlet{
public:
	/**
	 * @brief 类型定义，智能指针
	 */
	typedef std::shared_ptr<HtmlServlet> ptr;

	/**
	 * @brief 构造函数
	 * @param[in] path html的根路径
	 */
	HtmlServlet(const std::string & path);
	
	/**
	 * @brief 重写handle,加载html文件写进body
	 */
	void handle(HttpResponce::ptr res)override;
private:
	//html根路径
	std::string m_path ; 
};

}

#endif 
