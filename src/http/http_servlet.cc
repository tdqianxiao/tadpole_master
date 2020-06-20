#include "http_servlet.h"
#include "src/log.h"
#include "src/config.h"
#include <fstream>
#include <dirent.h>

namespace tadpole{

ConfigVar<std::string>::ptr g_html_root_path_conf = 
			Config::Lookup<std::string>("html.root.path","./html","html / path");

static std::string g_html_root_path ;

void travDir(const std::string & dirname,
			 std::function<void(const std::string & name)> cb = nullptr){
	DIR * dir= opendir(dirname.c_str());
	struct dirent * file ; 
	while((file = readdir(dir)) != nullptr){
		if(file->d_name[0] == '.'){
			continue; 
		}

		if(file->d_type == DT_DIR){
			travDir(dirname + "/" +std::string(file->d_name),cb);	
			continue;
		}
		if(cb){
			cb(dirname + "/" +std::string(file->d_name));	
		}
	}
}

void func(const std::string& name){
	HtmlServlet::ptr htmlser(new HtmlServlet(name));
	std::string str(&name[0]+g_html_root_path.size(),name.size()-g_html_root_path.size());
	ServletMgr::GetInstance()->addServlet(str,htmlser);
	TADPOLE_LOG_INFO(TADPOLE_FIND_LOGGER("root")) <<str<<" - "<<name ; 
}

struct _HtmlIniter{
	_HtmlIniter(){
		g_html_root_path_conf->addListener([](const std::string & old_val,
						const std::string & new_val){
							g_html_root_path = new_val;	
						});	
		g_html_root_path = "./html";
		
		travDir(g_html_root_path,func);
	}
};

static _HtmlIniter __htmliniter;

FunctionServlet::FunctionServlet(callback cb)
	:m_cb(cb){
}

void FunctionServlet::handle(HttpResponce::ptr res) {
	m_cb(res);
}

ServletDistributor::ServletDistributor(Servlet::ptr def){
	if(def){
		m_def = def;
	}else {
		m_def.reset(new NotFoundServlet);
	}
}

void ServletDistributor::addServlet(const std::string & uri,
									Servlet::ptr servlet){
	m_servlets.insert(std::make_pair(uri,servlet));	
}

void ServletDistributor::delServlet(const std::string & uri){
	m_servlets.erase(uri);
}

void ServletDistributor::addGlobServlet(const std::string & uri,
						Servlet::ptr servlet){
	m_globServlets.push_back(std::make_pair(uri,servlet));						
}

void ServletDistributor::delGlobServlet(const std::string & uri){
	for(auto it = m_globServlets.begin(); 
			it != m_globServlets.end(); ++it){
		if(it->first == uri){
			m_globServlets.erase(it);
			return ; 
		}
	}
}

Servlet::ptr ServletDistributor::getServlet(const std::string & uri){
	auto it = m_servlets.find(uri);
	if(it == m_servlets.end()){
		for(auto &it : m_globServlets){
			if(strncmp(&uri[0],&it.first[0],it.first.size()) == 0){
				return it.second;
			}
		}

		return m_def;
	}
	return it->second;
}

void NotFoundServlet::handle(HttpResponce::ptr res){
	std::string body  = "<html><head><title>404 Not Found"
        			  "</title></head><body><center><h1>404 Not Found</h1>"
					  "</center><hr><center>tadpole/1.0.0"
					  "</center></body></html>";
	res->setBody(body);
}

HtmlServlet::HtmlServlet(const std::string & path)
	:m_path(path){
}

void HtmlServlet::handle(HttpResponce::ptr res){
	std::shared_ptr<char> buffs(new char[4096],[](const char * buffs){
		delete[] buffs;
	});

	FILE* file = fopen(m_path.c_str(),"r+b"); 
	
	fseek(file,0,SEEK_END);
	int size = ftell(file);
	fseek(file,0,SEEK_SET);
	//TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root")) << "size : "<< size;	
	std::string str;
	str.reserve(size);
	//TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root")) << "cap : "<< str.capacity();	
	while(1){
		memset(buffs.get(),0,4096);
		int ret = fread(buffs.get(),1,4095,file);
		if(ret == 0){
			//TADPOLE_LOG_DEBUG(TADPOLE_FIND_LOGGER("root")) << "ret : "<<ret;	
			break; 
		}	
		str.append(buffs.get());

		if(ftell(file) >= size){
			break;
		}
	}
	fclose(file);	
	res->setBody(std::move(str));
}
}
