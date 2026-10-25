#include <iostream>
#include <thread>
#include <chrono>
#include "Command.h"
#include "state_machine.h"
#include "exec.h"

int main()
{
	
	while(true)
	{
		std::cout<<"mshell> ";
		
		std::string input;
		std::vector<Command> cmds;
		
		std::getline(std::cin, input);

		std::vector<std::string> raw_commands;

		int pos = 0;
		int del = 0;

		while((del = input.find("|", pos)) != std::string::npos)
		{
			raw_commands.push_back(input.substr(pos, (del - pos)));
			pos = del+1;
		}
		raw_commands.push_back(input.substr(pos));

		for(int i = 0; i< raw_commands.size(); i++)
		{
			cmds.push_back(command_preparation(raw_commands[i]));
			if(raw_commands.size() > 1 && i !=(raw_commands.size() - 1))
				cmds[i].pipe = true;
		}

		execute_command(cmds);

		clear_args(cmds);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
