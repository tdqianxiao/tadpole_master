#ifndef __TADPOLE_RANDOM_H__
#define __TADPOLE_RANDOM_H__

#include <memory>
#include <time.h>

#include "src/singleton.h"

namespace tadpole{

class Random{
public:
	//获得无序的1到54编号的数组
	static std::shared_ptr<uint8_t> GetCard54();
};

}

#endif 
