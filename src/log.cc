#include "log.h"

namespace tadpole{

//%d 输出日志时间点的日期或时间
//%F 输出日志信息所在的文件
//%l 输出日志事件的发生位置，即输出日志信息的语句处于它所在的类的第几行
//%t 输出产生该日志事件的线程名
//%f 输出日志所在的协程
//%p 输出优先级，即DEBUG，INFO，WARN，ERROR，FATAL。如果是调用debug()输出的，则为DEBUG，依此类推
//%c 输出日志信息所属的类的全名
//%m 输出代码中指定的信息，如log(message)中的message
//%n 输出一个回车换行符，Windows平台为“rn”，Unix平台为“n”
//%e 输出当前程序运行开始到现在的毫秒数
//%% 直接输出%

//formatItem 派生类

class DateFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<DateFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level, 
						LogEvent::ptr event)override {
	char mytime[32] = {0};
	time_t tv = event->getTime();
	strftime(mytime,sizeof(mytime),"%Y-%m-%d %H:%M:%S",localtime(&tv));
	os << mytime;
	}
};

class FileFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<FileFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getFilename();
	}
};

class LineFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<LineFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getLine();
	}
};

class ThreadFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<ThreadFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getThreadId();
	}
};


class FiberFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<FiberFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getFiberId();
	}
};

class LevelFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<LevelFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << LogLevel::ToString(level);
	}
};

class NameFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<NameFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getLoggerName();
	}
};

class MessageFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<MessageFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getContent();
	}
};

class NewLineFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<NewLineFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << std::endl;
	}
};

class TableFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<TableFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << "\t";
	}
};

class StringFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<StringFormatItem> ptr;
	
	StringFormatItem(const std::string & str)
		:m_str(str){
	}
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << m_str;
	}

private:
	std::string  m_str;
};

class ElapseFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<ElapseFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << event->getElapse();
	}
};

class PercentFormatItem:public LogFormatter::FormatItem{
public:
	typedef std::shared_ptr<PercentFormatItem> ptr;
	
	void format(std::ostream & os , LogLevel::Level level ,
						LogEvent::ptr event)override {
		os << "%";
	}
};

//end formatItem 

EventWrap::EventWrap(Logger::ptr logger ,LogLevel::Level level,
					 LogEvent::ptr event)
	:m_logger(logger)
	,m_event(event)
	,m_level(level){
}

std::stringstream & EventWrap::getSS(){
	return m_event->getSS();
}

EventWrap::~EventWrap(){
	m_logger->log(m_level,m_event);
}

std::string LogLevel::ToString(LogLevel::Level level){
	switch(level){
	case LogLevel::DEBUG:
		return "DEBUG";
	case LogLevel::INFO:
		return "INFO";
	case LogLevel::WARN:
		return "WARN";
	case LogLevel::ERROR:
		return "ERROR";
	case LogLevel::FATAL:
		return "FATAL";
	default : 
		return "UNKNOW";
	}
}

LogLevel::Level LogLevel::FromString(std::string val){
	std::transform(val.begin(),val.end(),val.begin(),::tolower);
#define XX(name)\
	if(val == #name){\
		return LogLevel::name;\
	}

	XX(DEBUG)
	XX(INFO)
	XX(WARN)
	XX(ERROR)
	XX(FATAL)
#undef XX
	return LogLevel::UNKNOW;
}

LogEvent::LogEvent(uint32_t line,uint32_t threadid,
			 uint32_t fiberid, uint64_t time,
			 uint64_t elapse,const std::string & filename
			 ):m_line(line)
			  ,m_threadId(threadid)
			  ,m_fiberId(fiberid)
			  ,m_time(time)
			  ,m_elapse(elapse)
			  ,m_filename(filename){}

Logger::Logger(const std::string & name)
	:m_name(name){
}

void Logger::log(LogLevel::Level level , LogEvent::ptr event){
	event->setLoggerName(m_name);
	if(level >= m_level){	
		for(auto & it : m_logAppenders){
			if(level >= it->getLevel()){
				it->format(level,event);
			}
		}
	}
}

void Logger::addAppender(LogAppender::ptr append){
	if(append){
		m_logAppenders.push_back(append);
	}
}

void Logger::delAppender(LogAppender::ptr append){
	for(auto it = m_logAppenders.begin(); it != m_logAppenders.end();
		++it){
		if(*it == append){
			m_logAppenders.erase(it);
		}
	}
}

void Logger::debug(LogEvent::ptr event){
	log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event){
	log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event){
	log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event){
	log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event){
	log(LogLevel::FATAL, event);
}

void LogAppender::setFormat(LogFormatter::ptr format){
	m_logFormatter= format;
}

void LogAppender::setFormat(const std::string & format){
	m_logFormatter.reset(new LogFormatter(format));
}

LogAppender::LogAppender(){
	m_logFormatter.reset(new LogFormatter());
}

StdoutLogAppender::StdoutLogAppender(){
}

StdoutLogAppender::~StdoutLogAppender(){
}

void StdoutLogAppender::format(LogLevel::Level level,LogEvent::ptr event){
	if(m_logFormatter){
		std::cout<<m_logFormatter->format(level,event);
	}
}

FileLogAppender::FileLogAppender(const std::string & name)
	:m_filename(name){
	reopen();
}
bool FileLogAppender::reopen(){
	if(m_fileStream.is_open()){
		m_fileStream.close();
	}

	m_fileStream.open(m_filename);

	return !!m_fileStream;
}
FileLogAppender::~FileLogAppender(){
	m_fileStream.close();
}

void FileLogAppender::format(LogLevel::Level level, LogEvent::ptr event) {
	if(m_logFormatter){
		m_fileStream << m_logFormatter->format(level,event);
	}
}

std::string LogFormatter::format(LogLevel::Level level, LogEvent::ptr event){
	std::stringstream ss; 
	ss.clear();
	for(auto & it : m_formatItems){
		it->format(ss,level,event);	
	}
	return ss.str();
}

void LogFormatter::init(){
	//first type 0-string , 1 format , second format 
	std::vector<std::pair<int , std::string>> vec;
	
	std::string str;
	std::string fstr; 
	int f = 0 ;
	int n = 0 ; 
	for(int i = 0 ; i < (int)m_pattern.size() ; ++i){
		if(m_pattern[i] != '%'){
			continue; 
		}
		n = i+1;
		str = m_pattern.substr(f,i-f);
		if(n < (int)m_pattern.size()){
				fstr = m_pattern.substr(n,1);
				f = n + 1 ; 
				i = n ;
		}
		if(!str.empty()){
			vec.push_back(std::make_pair(0,str));
			str.clear();
		}
		vec.push_back(std::make_pair(1,fstr));
		fstr.clear();
	}
	str = m_pattern.substr(f,m_pattern.size() - f);
	vec.push_back(std::make_pair(0,str));

	static std::map<std::string , std::function<FormatItem::ptr()> > s_fmt_map_str = {
#define XX(fmt,item) \
	std::make_pair(\
		fmt,[]()->FormatItem::ptr{\
			return item::ptr(new item());\
		}\
	),
	
	XX("d",DateFormatItem)
	XX("F",FileFormatItem)
	XX("l",LineFormatItem)
	XX("t",ThreadFormatItem)
	XX("f",FiberFormatItem)
	XX("p",LevelFormatItem)
	XX("c",NameFormatItem)
	XX("m",MessageFormatItem)
	XX("n",NewLineFormatItem)
	XX("T",TableFormatItem)
	XX("e",ElapseFormatItem)
	XX("%",PercentFormatItem)
#undef XX
	};

	for(auto & it : vec){
		if(it.first == 0){
			m_formatItems.push_back(StringFormatItem::ptr(new StringFormatItem(it.second)));
		}else{
			auto item = s_fmt_map_str.find(it.second);
			if(item != s_fmt_map_str.end()){
				m_formatItems.push_back(item->second());
			}else {
				m_formatItems.push_back(StringFormatItem::ptr(new StringFormatItem("<error>")));
			}
		}
	}
}

LogFormatter::LogFormatter(const std::string& pattern)
	:m_pattern(pattern){
	if(m_pattern.empty()){
		m_pattern = "%d%T%e%T[%c]%T[%p]%T[%F:%l]%T[%t:%f]%T%m%n";//默认输出格式
	}
	init();
}

Logger::ptr LoggerMgr::getLogger(const std::string & name){
	auto it = m_loggers.find(name);
	if(it == m_loggers.end()){
		auto logger = Logger::ptr(new Logger(name));
		logger->addAppender(StdoutLogAppender::ptr(new StdoutLogAppender));
		m_loggers.insert(make_pair(name,logger));
		return logger;
	}
	return it->second;
}

}

