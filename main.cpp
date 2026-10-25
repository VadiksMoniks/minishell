#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <string>
/**
ОБРАБОТКА КАВЫЧЕК
УДАЛЕНИЕ ПРОБЕЛОВ ДО И ПОСЛЕ ПАЙПА |
*/
struct Command{
	std::vector<char*> argv;
	std::string filename;
	int flags = 0;	
	bool pipe = false;
};

void clear_args(std::vector<Command>& cmds)
{
	for(int i = 0; i<cmds.size(); i++)
	{
		for(int j =0; j<cmds[i].argv.size(); j++)
		{
			if(cmds[i].argv[j] != nullptr)
				free(cmds[i].argv[j]);
		}
	}
}

const char* internal_commands[] = {
	"exit",
	"cd"
};

std::string read_token(const char* data, char delimeter) { 
    std::string token; 
    while(*data != delimeter && *data != '\0') { 
   		token += *data; 
   		data += 1;	
    } 
    return token; 
}

Command command_preparation(std::string& raw_command) {
	Command cmd; 
    int i = 0; 
    int n = raw_command.size();

    while (i < n) {        
        if (i >= n) break;

        std::string token = read_token(raw_command.data() + i, ' ');
        i += token.size() + 1;//skip whitespace

        if (token == ">") { 
            cmd.flags |= O_CREAT | O_WRONLY | O_TRUNC; 
            
            while (i < n && raw_command[i] == ' ') i++;
            
            cmd.filename = read_token(raw_command.data() + i, ' '); 
            i += cmd.filename.size() + 1; 
        } 
        else if (token == ">>") { 
            cmd.flags |= O_CREAT | O_WRONLY | O_APPEND; 
            
            while (i < n && raw_command[i] == ' ') i++;
            
            cmd.filename = read_token(raw_command.data() + i, ' '); 
            i += cmd.filename.size() + 1; 
        } 
        else if (token == "<") { 
            cmd.flags |= O_RDONLY; 
            
            while (i < n && raw_command[i] == ' ') i++;
            
            cmd.filename = read_token(raw_command.data() + i, ' '); 
            i += cmd.filename.size() + 1; 
        } 
        else { 
            cmd.argv.push_back(strdup(token.data())); 
        } 
    } 
	cmd.argv.push_back(NULL);

	return cmd;
}


void execute_external_command(Command& cmd)
{
	pid_t pid = fork();
	if(pid <0)
	{
		std::cout<<"Fork failed\n";
	}
	else if(pid == 0)
	{

		int fd;
		if(cmd.flags != 0)
		{

			fd = open(cmd.filename.data(), cmd.flags, 0644);
			int access_flag = cmd.flags & O_ACCMODE;

			if(access_flag == O_WRONLY)
				dup2(fd, STDOUT_FILENO);
			else if(access_flag == O_RDONLY)
				dup2(fd, STDIN_FILENO);
		}
		execvp(cmd.argv[0], cmd.argv.data());
		
		exit(EXIT_FAILURE);

		if(cmd.flags != 0)
			close(fd);
			
	}
	else if(pid > 0)
	{
		wait(NULL);
	}
}

void execute_pipeline(std::vector<Command>& cmds)
{
	int prev_descriptor = -1;
	std::vector<pid_t> pids;

	for(int i = 0; i<cmds.size(); i++)
	{
		int fd[2];
		bool has_next = i + 1 < cmds.size();

		if(has_next)
			pipe(fd);

		pid_t pid = fork();

		if(pid == 0)
		{
			if(prev_descriptor != -1)
			{
				dup2(prev_descriptor, STDIN_FILENO);
			}

			if(has_next)
			{
				dup2(fd[1], STDOUT_FILENO);
			}

			if(prev_descriptor != -1)
			{
				close(prev_descriptor);
			}

			if(has_next)
			{
				close(fd[0]);
				close(fd[1]);
			}

			execvp(cmds[i].argv[0], cmds[i].argv.data());
			exit(EXIT_FAILURE);
		}
		else if(pid > 0)
		{
			pids.push_back(pid);

			if(prev_descriptor != -1)
			{
				close(prev_descriptor);
			}
			if(has_next)
			{
				close(fd[1]);
				prev_descriptor = fd[0];
			}
		}
	}

	for(pid_t pid : pids)
		waitpid(pid, nullptr, 0);
}

bool execute_internal_command(Command& cmd)
{
	if(std::strcmp(cmd.argv[0], "exit") == 0)
		exit(0);

	if((std::strcmp(cmd.argv[0], "cd") == 0) && cmd.argv.size() >= 2)
	{	
		chdir(cmd.argv[1]);
		return true;
	}

	return false;
}

/*
** МОГУТ БЫТЬ ПРОБЛЕМЫ ПРИ ПАРСИНГЕ, ЕСЛИ НАПИСАТЬ | В КОНЦЕ СТРОКИ ИЛИ ПОСЛЕ ПАЙПА ЕЩЕ ДОБАВИТЬ ПРОБЕЛ В КОНЦЕ СТРОКИ,
** ВОЗМОЖНО ПРОГРАММА ПОСЧИТАЕТ ЧТО ЕСТЬ ЕЩЕ ОДНА КОМАНДА
*/
void execute_command(std::vector<Command>& cmds)
{	
		if(cmds.size() > 1)
		{
			execute_pipeline(cmds);	
		}
		else{
			if(execute_internal_command(cmds[0]) == false)
				execute_external_command(cmds[0]);
			else
				return;
		}
}

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

	//	for(int i =0; i<cmds.size(); i++)
			execute_command(cmds);

		clear_args(cmds);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
