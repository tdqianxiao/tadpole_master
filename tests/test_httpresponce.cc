#include "src/http/http_responce.h"
#include "src/log.h"
#include "src/socket.h"
#include "src/bytearray.h"

using namespace tadpole;

static Logger::ptr g_logger = TADPOLE_FIND_LOGGER("root");

char buf[] ="HTTP/1.0 200 OK\r\n"
			"Accept-Ranges: bytes\r\n"
			"Cache-Control: no-cache\r\n"
			"Content-Length: 14615\r\n"
			"Content-Type: text/html\r\n"
			"Date: Thu, 04 Jun 2020 09:51:14 GMT\r\n"
			"P3p: CP= \" OTI DSP COR IVA OUR IND COM \"\r\n"
			"Pragma: no-cache\r\n"
			"Server: BWS/1.1\r\n";

int main (){
	TADPOLE_LOG_INFO(g_logger)<< buf ; 
	HttpParserResponce::ptr hp(new HttpParserResponce);
	ByteArray::ptr byte(new ByteArray);

	byte->write(buf,strlen(buf)/2);
	std::string str = byte->moveToString(strlen(buf)/2);

	byte->write(buf + strlen(buf)/2, strlen(buf) - strlen(buf)/2);
	std::string str1 = byte->moveToString(strlen(buf) - strlen(buf)/2);

	 hp->execute(str.c_str(),str.size());
	hp->execute(str1.c_str(),str1.size());
	hp->getResponce()->setStatus((http_status)404);
	TADPOLE_LOG_INFO(g_logger)<< hp->getResponce()->toString();
	return 0 ; 
}
