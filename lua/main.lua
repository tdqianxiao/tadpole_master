require "lua.iom"

function test()
	log_info("successful")
end 

function main()
	print("main begin")
	local iom = IOManager.new(4)
	iom:schedule("test");

	iom:add_timer(2000,"test");
	sleep(3)
	log_info("sleep")
end 
