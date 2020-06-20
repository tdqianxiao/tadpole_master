#include "src/bytearray.h"
#include "src/macro.h"
#include "src/log.h"
#include <string.h>

namespace tadpole{

ByteArray::ByteArray(size_t size)
	:m_pos(nullptr)
	,m_cur(nullptr)
	,m_size(0)
	,m_basesize(size)
	,m_capcity(size)
	,m_root(new Node(m_basesize)){
		m_pos = m_root->buf; 
		m_cur = m_root->buf;
		m_curWriteNode = m_root;
}

ByteArray::~ByteArray(){
	if(m_root){
		while(m_root->next){
			Node * tmp = m_root->next; 
			delete m_root;
			m_root = tmp; 
		}
	}
	freeReadBuffers();
}

//写一个buf 
void ByteArray::write(const char * buf , size_t size){
	//当前节点剩余的容量
	size_t last_size = 0;
	while((last_size = useCapcity()) < size ){
		//先把前面的考上去
		memcpy(m_pos,buf,last_size);
		//判断当前节点的下一个节点是否已经分配	
		if(m_curWriteNode->next == nullptr){
			m_curWriteNode->next = new Node(m_basesize);
			m_capcity += m_basesize;	
		}
		//当前可写节点指向下一个节点
		m_curWriteNode = m_curWriteNode->next;
		m_pos = m_curWriteNode->buf; 

		m_size += last_size;
		buf += last_size;
		size -= last_size ; 
	}
	memcpy(m_pos,buf,size);
	m_pos += size; 
	m_size += size; 
}

//读buf,若size大于buf里面的size 则直接返回
bool ByteArray::read(char * buf , size_t size){
	if(size > m_size){
		return false; 
	}
	size_t last_size =  0;
	while((last_size = readLastSize()) < size){
		memcpy(buf,m_cur,last_size);

		//释放已经读完的节点
		Node * tmp = m_root;
		m_root = m_root->next;
		delete tmp ; 
		m_cur = m_root->buf;
		m_capcity -= last_size;
		m_size -= last_size;
		
		size -= last_size;
		buf += last_size;
	}
	memcpy(buf,m_cur,size);
	m_cur += size; 

	m_capcity -= size ; 
	m_size -= size;
	return true;
}

void ByteArray::writeFInt8(int8_t val){
	write((char *)&val,1);
}

void ByteArray::writeFInt16(int16_t val){
	val = byteswapOnBigEndian(val);
	write((char *)&val,2);
}

void ByteArray::writeFInt32(int32_t val){
	val = byteswapOnBigEndian(val);
	write((char *)&val,4);
}

void ByteArray::writeFInt64(int64_t val){
	val = byteswapOnBigEndian(val);
	write((char *)&val,8);
}

void ByteArray::writeFUint8(uint8_t val){
	write((char *)&val,1);
}

void ByteArray::writeFUint16(uint16_t val){
	val = byteswapOnBigEndian(val);
	write((char *)&val,2);
}

void ByteArray::writeFUint32(uint32_t val){
	val = byteswapOnBigEndian(val);
	write((char *)&val,4);
}

void ByteArray::writeFUint64(uint64_t val){
	val = byteswapOnBigEndian(val);
	write((char *)&val,8);
}

void ByteArray::writeInt32(int32_t val){	
	char tmp[5] = {0};
	int n = 0;
	while(val){
		tmp[n] = val & 0x7f;
		++n;
		val = val >> 7 ; 
	}
	tmp[n-1] |= 0x80; 
	write(tmp,n);
}

void ByteArray::writeInt64(int64_t val){
	char tmp[10] = {0};
	int n = 0;
	while(val){
		tmp[n] = val & 0x7f;
		++n;
		val = val >> 7 ; 
	}
	tmp[n-1] |= 0x80; 
	write(tmp,n);
}

void ByteArray::writeUint32(uint32_t val){
	writeInt32(val);
}

void ByteArray::writeUint64(uint64_t val){
	writeInt64(val);
}

void ByteArray::readFInt8(int8_t &val){
	read((char *)&val,1); 
}

void ByteArray::readFInt16(int16_t &val){
	read((char *)&val,2);
	val = byteswapOnBigEndian(val);
}

void ByteArray::readFInt32(int32_t &val){
	read((char *)&val,4);
	val = byteswapOnBigEndian(val);
}

void ByteArray::readFInt64(int64_t &val){
	read((char *)&val,8);
	val = byteswapOnBigEndian(val);
}

void ByteArray::readFUint8(uint8_t &val){
	read((char *)&val,1);
}

void ByteArray::readFUint16(uint16_t &val){
	read((char *)&val,2);
	val = byteswapOnBigEndian(val);
}

void ByteArray::readFUint32(uint32_t &val){
	read((char *)&val,4);
	val = byteswapOnBigEndian(val);
}

void ByteArray::readFUint64(uint64_t &val){
	read((char *)&val,8);
	val = byteswapOnBigEndian(val);
}

void ByteArray::readInt32(int32_t &val) {
	val = 0;
	char tmp = 0;
	int n = 0; 
	while (true) {
		read(&tmp, 1);
		if (tmp & 0x80) {
			tmp &= ~0x80;
			val |= tmp<<(n*7);
			break;
		}
		val |= tmp<<(n*7);
		++n;
	}
}

void ByteArray::readInt64(int64_t &val) {
	val = 0;
	char tmp = 0;
	int n = 0;
	while (true) {
		read(&tmp, 1);
		if (tmp & 0x80) {
			tmp &= ~0x80;
			val |= tmp << (n*7);
			break;
		}
		val |= tmp<<(n*7);
		++n;
	}
}

void ByteArray::readUint32(uint32_t &val) {
	val = 0;
	char tmp = 0;
	int n = 0;
	while (true) {
		read(&tmp, 1);
		if (tmp & 0x80) {
			tmp &= ~0x80;
			val |= tmp << (n * 7);
			break;
		}
		val |= tmp << (n * 7);
		++n;
	}
}

void ByteArray::readUint64(uint64_t &val) {
	val = 0;
	char tmp = 0;
	int n = 0;
	while (true) {
		read(&tmp, 1);
		if (tmp & 0x80) {
			tmp &= ~0x80;
			val |= tmp << (n * 7);
			break;
		}
		val |= tmp << (n * 7);
		++n;
	}
}
	
void ByteArray::writeString(const std::string & str){
	size_t i = str.size();
	writeFUint32(i);
	write(&str[0],i);
}

void ByteArray::readString(std::string & str){
	uint32_t size = 0 ;
	readFUint32(size);
	str.resize(size);
	read(&str[0],size);
}


void ByteArray::writeFloat(const float& val){
	uint32_t tmp = 0 ; 
	memcpy(&tmp,&val,4);
	writeFUint32(tmp);
}

void ByteArray::writeDouble(const double& val){
	uint64_t tmp = 0 ; 
	memcpy(&tmp,&val,8);
	writeFUint64(tmp);
}

void ByteArray::readFloat(float &val){
	uint32_t tmp = 0 ; 
	readFUint32(tmp);
	memcpy(&val,&tmp,4);
}

void ByteArray::readDouble(double &val){
	uint64_t tmp = 0 ; 
	readFUint64(tmp);
	memcpy(&val,&tmp,8);
}

//获得可读buf,只加容量，不加大小
uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffs , uint64_t size){
	//当前节点的可用大小
	size_t last_size = useCapcity();
	//返回值
	uint64_t n = 0 ; 
	Node * tmp = nullptr;
	if(last_size < size ){
		iovec iov ; 
		iov.iov_base = m_pos;
		iov.iov_len = last_size;
		buffs.push_back(iov);
		n += iov.iov_len;
		size -= last_size ;

		if(m_curWriteNode->next == nullptr){
			m_curWriteNode->next = new Node(m_basesize);
			m_capcity += m_basesize;
		}
		tmp = m_curWriteNode->next;

		while(size > m_basesize){
			iovec iov ;
			iov.iov_base = tmp->buf;
			iov.iov_len = m_basesize;
			n += iov.iov_len;

			buffs.push_back(iov);
			size -= m_basesize; 

			if(tmp->next == nullptr){
				tmp->next = new Node(m_basesize);
				m_capcity += m_basesize;
			}
			tmp = tmp->next;
		
		}
	}
	iovec iov ;
	iov.iov_base = tmp ? tmp->buf : m_pos;
	iov.iov_len = size;
	buffs.push_back(iov);
	n += iov.iov_len;
	return n;
}

void ByteArray::setUsedCount(uint64_t size){
	//获得当前buf还有多少容量
	m_size += size;

	size_t last_size = useCapcity();
	bool is_out_of = false; 
	if(last_size < size){
		m_curWriteNode = m_curWriteNode->next ; 
		size -= last_size;
		while(size > m_basesize){
		m_curWriteNode = m_curWriteNode->next; 
		size -= m_basesize;
		}
		is_out_of = true; 
	}

	m_pos = is_out_of ? (m_curWriteNode->buf + size) : (m_pos + size); 
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffs, uint64_t size){
	if(size > m_size){
		return 0 ; 
	}
	int n = 0 ; 
	size_t last_size = 0;
	while((last_size = readLastSize()) < size){
		iovec iov ; 
		iov.iov_base = m_cur ; 
		iov.iov_len = last_size; 
		buffs.push_back(iov);
		
		m_nodes.push_back(m_root);
		m_root = m_root->next;
		
		m_cur = m_root->buf;
		m_capcity -= last_size;
		m_size -= last_size;

		size -= last_size;
		n += last_size;
	}
	iovec iov ; 
	iov.iov_base = m_cur ; 
	iov.iov_len = size; 
	buffs.push_back(iov);
	
	n += last_size;
	m_cur += size; 
	m_capcity -= size ; 
	m_size -= size;
	return n ; 
}

void ByteArray::freeReadBuffers(){
	for(auto & it : m_nodes){
		if(it){
			delete it; 
		}
	}
	m_nodes.clear();
}

}
