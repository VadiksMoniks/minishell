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

struct PassedParams{
	std::vector<char*> argv;
	std::string filename;
	int flags = 0;	
	bool pipe = false;
};

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

void input_processing(std::string& command, PassedParams& params) { 
    int i = 0; 
    int n = command.size();

    while (i < n) {
        while (i < n && command[i] == ' ') {
            i++;
        }
        if (i >= n) break;

        std::string token = read_token(command.data() + i, ' ');
        i += token.size();

        if (token == ">") { 
            params.flags |= O_CREAT | O_WRONLY | O_TRUNC; 
            
            while (i < n && command[i] == ' ') i++;
            
            params.filename = read_token(command.data() + i, ' '); 
            i += params.filename.size(); 
        } 
        else if (token == ">>") { 
            params.flags |= O_CREAT | O_WRONLY | O_APPEND; 
            
            while (i < n && command[i] == ' ') i++;
            
            params.filename = read_token(command.data() + i, ' '); 
            i += params.filename.size(); 
        } 
        else if (token == "<") { 
            params.flags |= O_RDONLY; 
            
            while (i < n && command[i] == ' ') i++;
            
            params.filename = read_token(command.data() + i, ' '); 
            i += params.filename.size(); 
        } 
        else { 
            params.argv.push_back(strdup(token.data())); 
        } 
    } 
	params.argv.push_back(NULL);
}


void execute_external_command(PassedParams& params)
{
	pid_t pid = fork();
	if(pid <0)
	{
		std::cout<<"Fork failed\n";
	}
	else if(pid == 0)
	{

		int fd;
		if(params.flags != 0)
		{

			fd = open(params.filename.data(), params.flags, 0644);
			int access_flag = params.flags & O_ACCMODE;

			if(access_flag == O_WRONLY)
				dup2(fd, STDOUT_FILENO);
			else if(access_flag == O_RDONLY)
				dup2(fd, STDIN_FILENO);
		}
		execvp(params.argv[0], params.argv.data());
		
		exit(EXIT_FAILURE);

		if(params.flags != 0)
			close(fd);
			
	}
	else if(pid > 0)
	{
		wait(NULL);
	}
}

bool execute_internal_command(PassedParams& params)
{
	if(std::strcmp(params.argv[0], "exit") == 0)
		exit(0);

	if((std::strcmp(params.argv[0], "cd") == 0) && params.argv.size() >= 2)
	{	
		chdir(params.argv[1]);
		return true;
	}

	return false;
}

void execute_command(PassedParams& params)
{
		if(execute_internal_command(params) == false)
			execute_external_command(params);
		else
			return;
}

int find_command()
{
	return 0;
}

int main()
{
	
	while(true)
	{
		std::cout<<"mshell> ";
		
		std::string command;
		PassedParams params;
		
		std::getline(std::cin, command);

		input_processing(command, params);
		execute_command(params);

		for(int i =0; i<params.argv.size(); i++)
		{
			if(params.argv[i] != nullptr)
				free(params.argv[i]);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
