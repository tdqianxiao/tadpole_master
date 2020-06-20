#include <iostream>
#include "yaml.h"
#include "src/config.h"
#include "src/log.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void travnode(YAML::Node node,int level){
	if(node.IsMap()){
		for(auto it = node.begin(); it != node.end();++it){
			TADPOLE_LOG_INFO(g_logger) << std::string(level,' ')<< it->first;
			travnode(it->second,level + 2);
		}
	}else if(node.IsSequence()){
		for(auto it = node.begin() ; it != node.end();++it){
			travnode(*it,level +2);
		}
	}else if(node.IsScalar()){
		TADPOLE_LOG_INFO(g_logger) <<std::string(level,' ')<< node.Scalar();
	}
}

void test_yaml(){
	YAML::Node node = YAML::LoadFile("./config/log.yml");
	travnode(node,0);
}

void test_config(){

#define XXPRE(type,val,name,impl) \
	tadpole::ConfigVar<type>::ptr impl = tadpole::Config::Lookup<type>(name,val,name);\
	TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl->toString();\
	TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl->getValue();
#define XX(impl) \
	TADPOLE_LOG_INFO(g_logger)<<"last:"<<impl->toString();\
	TADPOLE_LOG_INFO(g_logger)<<"last:"<<impl->getValue();

#define PRINT(Seq) \
	for(auto &it : Seq){\
		TADPOLE_LOG_INFO(g_logger) << it; \
		}

#define PRINTM(Seq) \
	for(auto &it : Seq){\
		TADPOLE_LOG_INFO(g_logger) << it.first << " - "<< it.second; \
		}
XXPRE(uint32_t,30,"int.num",test1);

tadpole::ConfigVar<std::vector<int>>::ptr impl = tadpole::Config::Lookup<std::vector<int>>("Sequence.vec",{1,2,3},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl->toString();

tadpole::ConfigVar<std::list<int>>::ptr impl1 = tadpole::Config::Lookup<std::list<int>>("Sequence.list",{1,2,3},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl1->toString();

tadpole::ConfigVar<std::set<int>>::ptr impl2 = tadpole::Config::Lookup<std::set<int>>("Sequence.set",{1,5,3},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl2->toString();

tadpole::ConfigVar<std::map<std::string,int>>::ptr impl3 = tadpole::Config::Lookup<std::map<std::string,int>>("Sequence.map",{std::make_pair("key",1)},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl3->toString();

tadpole::ConfigVar<std::unordered_set<int>>::ptr impl4 = tadpole::Config::Lookup<std::unordered_set<int>>("Sequence.uset",{1,5,3},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl4->toString();

tadpole::ConfigVar<std::unordered_map<std::string,int>>::ptr impl5 = tadpole::Config::Lookup<std::unordered_map<std::string,int>>("Sequence.umap",{std::make_pair("key",1)},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl5->toString();

std::vector<int> temp = {17,8,10};
tadpole::ConfigVar<std::list<std::vector<int>>>::ptr impl6 = tadpole::Config::Lookup<std::list<std::vector<int>>>("Sequence.list_vec",{temp},"sequencevec");
TADPOLE_LOG_INFO(g_logger)<<"previous:"<<impl6->toString();

tadpole::Config::LoadFromYaml("./config/log.yml");
XX(test1);
TADPOLE_LOG_INFO(g_logger)<<"last:"<<impl->toString();
TADPOLE_LOG_INFO(g_logger)<<"last:"<<impl1->toString();
TADPOLE_LOG_INFO(g_logger)<<"last:"<<impl2->toString();
TADPOLE_LOG_INFO(g_logger)<<"last:"<<impl3->toString();

PRINT(impl->getValue());
PRINT(impl1->getValue());
PRINT(impl2->getValue());
PRINTM(impl3->getValue());
PRINT(impl4->getValue());
PRINTM(impl5->getValue());

for(auto &it : impl6->getValue()){
	for(auto &iter : it){
		TADPOLE_LOG_INFO(g_logger) << iter;
	}
}


#undef PRINT

#undef XXPRE
#undef XX
}

void cb(int o , int n ){	
	TADPOLE_LOG_INFO(g_logger)<<"old:" << o;
	TADPOLE_LOG_INFO(g_logger)<<"new:" << n;
}

void testlisten(){
	tadpole::ConfigVar<int>::ptr impl = tadpole::Config::Lookup<int>("Sequence.int",2,"sequencevec");
	TADPOLE_LOG_INFO(g_logger)<<impl->getValue();
	impl->addListener(cb);
	tadpole::Config::LoadFromYaml("./config/log.yml");
	TADPOLE_LOG_INFO(g_logger)<<impl->getValue();
}

int main (){
	//test_yaml();
	//test_config();
	testlisten();
}
