require "lua.class"

IOManager = class("Iomanager")

function IOManager:ctor(threadcount)
	self.m_iom = iom_create(threadcount,false,"hh");
end

function IOManager:schedule(func)
	iom_schedule(self.m_iom,func);
end 

function IOManager:add_timer(ms,func)
	iom_add_timer(self.m_iom,ms,func)
end 

function sleep(s)
	iom_sleep(s)
end 
