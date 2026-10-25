#pragma once

#include "Command.h"
#include <fcntl.h>
#include <sys/types.h>// pid_t
#include <iostream>// dup2
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>// wait
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