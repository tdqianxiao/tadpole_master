#include "src/log.h"
#include "src/config.h"
#include "yaml.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root"); 

void test_config(){
	TADPOLE_LOG_INFO(g_logger) << "hello";	
}

void test_node(){
	YAML::Node node ; 
	YAML::Node node1 ; 
	node1["name"] = "root";
	node.push_back(node1);
}

int main (){
	test_config();
	//test_node();
}
