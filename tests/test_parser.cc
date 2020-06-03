#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "src/http/http11_common.h"
#include "src/http/http11_parser.h"
#include "src/http/httpclient_parser.h"

#include "log.h"
#include "socket.h"
#include "bytearray.h"

static tadpole::Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

void element_cb_val(void *data, const char *at, size_t length){	
	TADPOLE_LOG_INFO(g_logger) <<" : "<< std::string(at,length);	
}

void field_cb_val(void *data, const char *field, size_t flen, const char *value, size_t vlen){
	TADPOLE_LOG_INFO(g_logger) <<"field : "<< std::string(field,flen);	
	TADPOLE_LOG_INFO(g_logger) <<"value : "<< std::string(value,vlen);	
}


using namespace tadpole;

void test_httpServer(){
	Socket::ptr sock = Socket::CreateTcpSocket();
	IPAddress::ptr addr = IPAddress::Create("127.0.0.1",80);
	sock->bind(addr);
	sock->listen();
	Socket::ptr cfd = sock->accept();

	ByteArray::ptr byte(new ByteArray());

	std::vector<iovec> vec;
	byte->getWriteBuffers(vec,4096);
	int ret = cfd->recv(vec);

	TADPOLE_LOG_INFO(g_logger) <<ret;
	if(ret <= 0 ){
		return ;
	}

	byte->setUsedCount(ret);
	vec.clear();

	byte->getReadBuffers(vec,ret);
	
	TADPOLE_LOG_INFO(g_logger) << std::string((char*)vec[0].iov_base,ret);
	struct http_parser parser;
	http_parser_init(&parser);

	parser.http_field = field_cb_val;
	parser.request_method = element_cb_val;

	parser.request_uri = element_cb_val;     
	parser.fragment    = element_cb_val;        
	parser.request_path= element_cb_val;    
	parser.query_string= element_cb_val;    
	parser.http_version= element_cb_val;    
	parser.header_done = element_cb_val;

    http_parser_execute(&parser, const_cast<const char *>((char*)vec[0].iov_base), ret,0);
}

int main(int argc, char** argv) {
  	test_httpServer(); 
}
