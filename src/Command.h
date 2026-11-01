#pragma once

#include <string>
#include <vector>

struct Command{
	std::vector<char*> argv;
	std::string filename;
	int flags = 0;	
	bool pipe = false;

	~Command()
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
};