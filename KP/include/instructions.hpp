#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "unistd.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "constants.hpp"
#include <string>

using namespace std;

struct user_info
{
    string username;
    int user_id;
};

struct message
{
    string author = "";
    string content = "";
    string channel = "";
};

void send_message(int pipe, message msg, query_id id)
{
    write(pipe, &id, sizeof(query_id));
    int len = msg.author.length();
    write(pipe, &len, sizeof(int));
    write(pipe, msg.author.c_str(), len);
    len = msg.content.length();
    write(pipe, &len, sizeof(int));
    write(pipe, msg.content.c_str(), len);
    len = msg.channel.length();
    write(pipe, &len, sizeof(int));
    write(pipe, msg.channel.c_str(), len);
}

message receive_msg(int pipe)
{
    message res;
    int n;
    read(pipe, &n, sizeof(int));
    char* str = (char*)calloc(n, sizeof(char));
    read(pipe, str, n);
    res.author = str;
    free(str);
    read(pipe, &n, sizeof(int));
    str = (char*)calloc(n, sizeof(char));
    read(pipe, str, n);
    res.content = str;
    free(str);
    read(pipe, &n, sizeof(int));
    str = (char*)calloc(n, sizeof(char));
    read(pipe, str, n);
    res.channel = str;
    free(str);
    return res;
}

struct group_modify
{
    string group = "";
    string username = "";
};

void send_group_modify(int pipe, group_modify mod, query_id id)
{
    write(pipe, &id, sizeof(query_id));
    int n = mod.group.length();
    write(pipe, &n, sizeof(int));
    write(pipe, mod.group.c_str(), n);
    n = mod.username.length();
    write(pipe, &n, sizeof(int));
    write(pipe, mod.username.c_str(), n);
}

group_modify receive_group_modify(int pipe)
{
    group_modify res;
    int n;
    read(pipe, &n, sizeof(int));
    char* str = (char*)calloc(n, sizeof(char));
    read(pipe, str, n);
    res.group = str;
    free(str);
    read(pipe, &n, sizeof(int));
    str = (char*)calloc(n, sizeof(char));
    read(pipe, str, n);
    res.username = str;
    free(str);
    return res;
}

#endif