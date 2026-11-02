#pragma once

#include <string>
#include <vector>
#include <cstring>

struct Command{
	std::vector<char*> argv;
	std::string filename;
	int flags = -1;	
	bool pipe = false;

	Command() = default;

	void clear() {
	    for (char* ptr : argv) {
	        if (ptr != nullptr) {
	            free(ptr);
	        }
	    }
	    argv.clear();
	    filename.clear();
	    flags = -1;
	    pipe = false;
	}
	
	Command(const Command& other)
	{	
		if(other.argv.size() > 0)
		{
			for(char* ptr : other.argv)
			{
				if(ptr != NULL)
					argv.push_back(strdup(ptr));
			}
			argv.push_back(NULL);

			if(!other.filename.empty())
				filename = other.filename;

			flags = other.flags;
			pipe  = other.pipe;
		}
	}

	Command& operator=(const Command& other)
	{
		if (this == &other) {
		    return *this; 
		}

		clear();
		
		if(other.argv.size() > 0)
		{
			for(char* ptr : other.argv)
			{
				if(ptr != NULL)
					argv.push_back(strdup(ptr));
			}
			argv.push_back(NULL);

			if(!other.filename.empty())
				filename = other.filename;

			flags = other.flags;
			pipe  = other.pipe;
		}

		return *this;
	}
	
	~Command()
	{
		clear();
	}
};
