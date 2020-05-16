#include "config.h"
#include "log.h"

namespace tadpole{

static std::map<std::string,std::string>& GetConfigItems(){
	static std::map<std::string , std::string> configItems;
	return configItems;
}

void Config::InitFromYaml(const YAML::Node & node,const std::string &prefix){
	if(node.IsMap()){
		for(auto it = node.begin(); it != node.end(); ++it){
			std::string slice = prefix.empty() ? "" : ".";
			std::string str = prefix + slice + it->first.Scalar();
			std::stringstream ss ; 
			ss << it->second;
			GetConfigItems().insert(std::make_pair(str,ss.str()));
			InitFromYaml(it->second,str);
		}	
	}else if(node.IsSequence()){
		for(auto it = node.begin(); it != node.end(); ++it){
			InitFromYaml(*it,prefix);
		}
	}else if (node.IsScalar()){
		std::stringstream ss;
		ss << node;
		GetConfigItems().insert(std::make_pair(prefix,ss.str()));
	}
}

void Config::LookupData(){
	auto &data = GetData();
	for(auto &it : GetConfigItems()){
		auto iter = data.find(it.first);
		if(iter != data.end()){
			iter->second->fromString(it.second);	
		}
	}
	GetConfigItems().clear();
}

bool Config::LoadFromYaml(const std::string & filename){
	try{
		YAML::Node node = YAML::LoadFile(filename);
		InitFromYaml(node,"");
		LookupData();	
		return true; 
	}catch(...){
		return false ; 
	}
}

}
