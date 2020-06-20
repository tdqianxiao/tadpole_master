#include "src/log.h"
#include "examples/FightLandlord/Random.h"

#include <vector>

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

int main(){
//	std::shared_ptr<uint8_t> cards = Random::GetCard54(true);
//	std::vector<uint8_t> vec;
//	for(int i = 0 ; i < 54 ; ++i){
///		if(i == 27){
//			std::cout<< std::endl;
//		}
//		std::cout<<(int)cards.get()[i]<< "  ";
//		vec.push_back(cards.get()[i]);
//	}

//	std::sort(vec.begin(),vec.end(),std::less<uint8_t>());

//	for(auto &it : vec){
//		TADPOLE_LOG_INFO(g_logger) << (int)it;
//	}
}
