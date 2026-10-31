#pragma once

#include "Command.h"
#include <cstring>
#include <fcntl.h>
#include <vector>

#define STATE_DEFAULT        0x1
#define STATE_DOUBLE_QUOTES  0x2
#define STATE_SINGLE_QUOTES  0x3

void flush_token(std::string& token, Command& cmd, int& redirect_type)
{
    if(!token.empty())
    {
        if(redirect_type == 1)
        {
            cmd.filename = token;
            redirect_type = 0; 
        }
            
        else
            cmd.argv.push_back(strdup(token.data()));

        token.clear();
    }   
}

std::vector<Command> parse(const std::string& input)
{
    std::vector<Command> cmds;
    Command cmd;
    std::string token;
    int current_state = STATE_DEFAULT;
    int redirect_type = 0;

    for(char c : input)
    {
        switch (current_state)
        {
            case STATE_DEFAULT:
                if(c == '\'')
                {
                    flush_token(token, cmd, redirect_type);
                    current_state = STATE_SINGLE_QUOTES;
                }
                else if(c == '\"')
                {
                    flush_token(token, cmd, redirect_type);
                    current_state = STATE_DOUBLE_QUOTES;
                }
                else if(c == '|')
                {
                    flush_token(token, cmd, redirect_type);
                    cmd.pipe = true;
                    cmd.argv.push_back(NULL);
                    cmds.push_back(cmd);
                    cmd = {};
                }
                else if(c == '>')
                {
                    flush_token(token, cmd, redirect_type);
                    if(redirect_type == 1)
                        cmd.flags = O_CREAT | O_WRONLY | O_APPEND;
                    else{
                        cmd.flags = O_CREAT | O_WRONLY | O_TRUNC; 
                        redirect_type = 1;  
                    }
                    
                }
                else if(c == '<')
                {
                    flush_token(token, cmd, redirect_type);
                    cmd.flags = O_RDONLY;
                    redirect_type = 1;
                }
                else if(c == ' ')
                {
                    flush_token(token, cmd, redirect_type);
                }
                else{
                    token += c;
                }
                break;

            case STATE_DOUBLE_QUOTES:
                if(c == '\"'){
                    current_state = STATE_DEFAULT;
                    flush_token(token, cmd, redirect_type);
                } 
                else
                    token += c;
                break;

            case STATE_SINGLE_QUOTES:
                if(c == '\''){
                   current_state = STATE_DEFAULT;
                    flush_token(token, cmd, redirect_type); 
                } 
                else
                    token += c;
                break;
            default:
                break;
        }
    }

    flush_token(token, cmd, redirect_type);

    if(!cmd.argv.empty())
    {
        cmd.argv.push_back(NULL);
        cmds.push_back(cmd);
    }

    return cmds;
}
