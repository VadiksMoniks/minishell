#pragma once

#include "Command.h"
#include <cstring>
#include <fcntl.h>//O_CREAT | O_WRONLY | O_TRUNC | O_APPEND

/**
ОБРАБОТКА КАВЫЧЕК
УДАЛЕНИЕ ПРОБЕЛОВ ДО И ПОСЛЕ ПАЙПА |
*/
/*
** МОГУТ БЫТЬ ПРОБЛЕМЫ ПРИ ПАРСИНГЕ, ЕСЛИ НАПИСАТЬ | В КОНЦЕ СТРОКИ ИЛИ ПОСЛЕ ПАЙПА ЕЩЕ ДОБАВИТЬ ПРОБЕЛ В КОНЦЕ СТРОКИ,
** ВОЗМОЖНО ПРОГРАММА ПОСЧИТАЕТ ЧТО ЕСТЬ ЕЩЕ ОДНА КОМАНДА
*/
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