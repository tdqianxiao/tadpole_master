#include "Random.h"
#include "src/log.h"
#include <string.h>

namespace tadpole{

std::shared_ptr<uint8_t> Random::GetCard54(){
	std::shared_ptr<uint8_t> cards(new uint8_t[54],[](const uint8_t *ptr){
		delete[] ptr;
	});
	uint8_t * buff = cards.get();
	memset(buff,0,54);
	srand(time(NULL));
	for(int i = 0 ; i < 54 ; ++i){
		int n = rand()%54;
		while(buff[n%54] != 0){
			++n;
		} 
		buff[n%54] = i;
	}
	return cards;
}

}
