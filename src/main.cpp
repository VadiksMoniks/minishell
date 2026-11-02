#include <iostream>
#include <thread>
#include <chrono>
#include "Command.h"
#include "exec.h"
#include "state_machineV2.h"

int main()
{
	
	while(true)
	{
		std::cout<<"mshell> ";
		
		std::string input;
		std::vector<Command> cmds;
		
		std::getline(std::cin, input);

		if(input.empty())
			continue;
			
		std::vector<std::string> raw_commands;

		cmds = parse(input);
		if(cmds.empty())
			continue;
			
		execute_command(cmds);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
