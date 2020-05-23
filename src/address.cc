#include "address.h"
#include "blbyteswap.h"

#include "log.h"
#include "util.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("system");

namespace tadpole{
	
int Address::getFamily(){
	return getAddr()->sa_family;
}
	
std::string Address::toString(){
	std::stringstream ss; 
	insert(ss);
	return ss.str();
}

bool Address::operator< (const Address & rhs)const{
	socklen_t min_len = std::min(getAddrLen(),rhs.getAddrLen());
	int result = memcmp(getAddr(),rhs.getAddr(),min_len);

	if(result < 0 ){
		return true;
	}else if(result > 0){
		return false;
	}else if(getAddrLen() < rhs.getAddrLen()){
		return true;
	}
	return true;
}
bool Address::operator== (const Address & rhs)const{
	return(getAddrLen() == rhs.getAddrLen() &&
	   	   memcmp(getAddr(),rhs.getAddr(),getAddrLen()));
}
bool Address::operator!= (const Address &rhs)const{
	return !(*this == rhs);
}

IPv4Address::IPv4Address(uint32_t ip, uint16_t port){
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = byteswapOnLittleEndian(port);
	m_addr.sin_addr.s_addr = byteswapOnLittleEndian(ip);
}


IPv4Address::ptr IPv4Address::Create(const char * ip,uint16_t port){
	IPv4Address::ptr ipv4(new IPv4Address());
	int ret = inet_pton(AF_INET,ip,&ipv4->m_addr.sin_addr.s_addr);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger)<<"create ipv4 error ! ip : "<<ip
								<<" port: "<<port<<" errno = "
								<<errno;
		return nullptr;
	}
	ipv4->m_addr.sin_port = byteswapOnLittleEndian(port);
	return ipv4;
}

const sockaddr* IPv4Address::getAddr()const  {
	return (sockaddr*)&m_addr;
}
socklen_t IPv4Address::getAddrLen()const  {
	return sizeof(m_addr);
}
std::ostream & IPv4Address::insert(std::ostream & os){
	uint32_t ip = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
	os<<"["<<(ip>>24)<< "."
			<<((ip>>16) & 0xff)<< "."
			<<((ip>>8) & 0xff)<< "."
			<<(ip & 0xff)<<"]:"
			<<byteswapOnLittleEndian(m_addr.sin_port);
	return os;
}
IPAddress::ptr IPv4Address::getBroadcast(uint32_t prefix)  {
	if(prefix > 32)return nullptr;
	uint32_t ip = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
	ip |= (1<<(32-prefix))-1;
	return IPv4Address::ptr(new IPv4Address(ip,byteswapOnLittleEndian(m_addr.sin_port)));
}

IPAddress::ptr IPv4Address::getNetwork(uint32_t prefix) {
	if(prefix > 32)return nullptr;
	uint32_t ip = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
	ip &= ~((1<<(32-prefix))-1);
	return IPv4Address::ptr(new IPv4Address(ip,byteswapOnLittleEndian(m_addr.sin_port)));
}

IPAddress::ptr IPv4Address::getSubnetmask(uint32_t prefix) {
	if(prefix > 32)return nullptr;
	int ip = ~((1<<(32-prefix)) - 1);
	return IPv4Address::ptr(new IPv4Address(ip,byteswapOnLittleEndian(m_addr.sin_port)));
} 

uint32_t IPv4Address::getPort()const {
	return byteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t port) {
	m_addr.sin_port = byteswapOnLittleEndian(port);
}

IPv6Address::IPv6Address(const sockaddr_in6 & addr){
	memcpy(&m_addr,&addr,sizeof(m_addr));
}

IPv6Address::IPv6Address(){
	memset(&m_addr,0,sizeof(m_addr));
}

IPv6Address::IPv6Address(const uint8_t * ip , uint16_t port ){	
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin6_family = AF_INET6;
	m_addr.sin6_port = byteswapOnLittleEndian(port);
	memcpy(m_addr.sin6_addr.s6_addr,ip,16);
}

IPv6Address::ptr IPv6Address::Create(const char * ip , uint16_t port){
	IPv6Address::ptr ipv6(new IPv6Address);
	int ret = inet_pton(AF_INET6,ip,&ipv6->m_addr.sin6_addr);
	if(ret == -1){
		TADPOLE_LOG_ERROR(g_logger) << "ipv6 Create no success !";
		return nullptr;
	}
	ipv6->m_addr.sin6_port = byteswapOnLittleEndian(port);
	ipv6->m_addr.sin6_family = AF_INET6;
	return ipv6;
}

const sockaddr* IPv6Address::getAddr()const  {
	return (sockaddr*)&m_addr;
}
socklen_t IPv6Address::getAddrLen()const  {
	return sizeof(m_addr);
}
std::ostream & IPv6Address::insert(std::ostream & os){
	uint16_t * ipv6 = (uint16_t*)m_addr.sin6_addr.s6_addr;
	os << "[";
	bool used_zero = false;
	for (int i = 0; i < 8; ++i) {
		if (ipv6[i] == 0 && !used_zero) {
			continue;

		}
		if (i && ipv6[i] && ipv6[i - 1] == 0 && !used_zero) {
			if (ipv6[0] == 0) {
				os << ":";
			}
			os << ":";
			os <<std::hex<< ipv6[i]<<std::dec;
			os << ":";
			used_zero = true;
			continue;

		}
		os <<std::hex<< ipv6[i]<<std::dec;
		if (i != 7) {
			os << ":";
		}

	}
	if(!used_zero){
		os<<":";
	}
	os << "]:"<<byteswapOnLittleEndian(m_addr.sin6_port);
	return os;
}
IPAddress::ptr IPv6Address::getBroadcast(uint32_t prefix)  {	
	if(prefix > 128)return nullptr;
	sockaddr_in6 sock6(m_addr);
	uint16_t * tempipv6 = (uint16_t *)sock6.sin6_addr.s6_addr;
	int mid = prefix/16;
	tempipv6[mid] = tempipv6[mid] | (uint16_t)((1<<(prefix%16))-1);
	for(int i = mid + 1 ; i < 8 ; ++i){
		tempipv6[i] = 0xffff;
	}
	return IPv6Address::ptr(new IPv6Address(sock6));
}
IPAddress::ptr IPv6Address::getNetwork(uint32_t prefix) {
	if(prefix > 128)return nullptr;
	sockaddr_in6 sock6(m_addr);
	uint16_t * tempipv6 =(uint16_t*)sock6.sin6_addr.s6_addr;
	int mid = prefix/16;
	tempipv6[mid] = tempipv6[mid] & ~(uint16_t)((1<<(prefix%16))-1);
	for(int i = mid + 1 ; i < 8 ; ++i){
		tempipv6[i] = 0;
	}
	return IPv6Address::ptr(new IPv6Address(sock6));
}
IPAddress::ptr IPv6Address::getSubnetmask(uint32_t prefix) {
	sockaddr_in6 sock6 = {0};
	sock6.sin6_port = m_addr.sin6_port;
	uint16_t * tempipv6 = (uint16_t*)sock6.sin6_addr.s6_addr;
	int mid = prefix/16;
	tempipv6[mid] = ~(uint16_t)((1<<(prefix%16))-1);
	for(int i = 0 ; i < mid ; ++i){
		tempipv6[i] = 0xffff;
	}
	return IPv6Address::ptr(new IPv6Address(sock6));
} 
uint32_t IPv6Address::getPort()const {
	return byteswapOnLittleEndian(m_addr.sin6_port);
}
void IPv6Address::setPort(uint16_t port) {
	m_addr.sin6_port = byteswapOnLittleEndian(port);
}

static const size_t MAX_PATH = sizeof(sockaddr_un) - offsetof(sockaddr_un,sun_path) -1;

UnixAddress::UnixAddress(){
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sun_family = AF_UNIX;
	m_length = offsetof(sockaddr_un,sun_path) + MAX_PATH;
}

UnixAddress::UnixAddress(const std::string & path){	
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sun_family = AF_UNIX;
	m_length = path.size() + 1;

	if(path.empty()){
		m_length = 0 ; 
	}
	if(path.size() >MAX_PATH){
		throw std::logic_error("unixAddress path is long !");
	}

	memcpy(m_addr.sun_path,path.c_str(),m_length);
	m_length += offsetof(sockaddr_un,sun_path);
}
const sockaddr* UnixAddress::getAddr()const  {
	return (sockaddr*)&m_addr;
}
socklen_t UnixAddress::getAddrLen()const  {
	return m_length;
}
std::ostream & UnixAddress::insert(std::ostream & os){
	os<<"[file:"<<m_addr.sun_path<<"]";
	return os;
}

UnKnowAddress::UnKnowAddress(){
}

const sockaddr* UnKnowAddress::getAddr()const  {
	return &m_addr;
}
socklen_t UnKnowAddress::getAddrLen()const  {
	return sizeof(m_addr);
}
std::ostream & UnKnowAddress::insert(std::ostream & os){
	os<<"[UnKnowAddress]";
	return os; 
}
}
