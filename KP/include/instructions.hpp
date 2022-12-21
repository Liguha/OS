#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include "unistd.h"
#include "sys/stat.h"
#include "fcntl.h"

#include "constants.hpp"
#include "check_err.hpp"

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

int send_message(int pipe, message msg, query_id id)
{
    int ok = 0;
    CHECK_ERROR(write(pipe, &id, sizeof(query_id)), "", ok = -1);
    int len = msg.author.length();
    CHECK_ERROR(write(pipe, &len, sizeof(int)), "", ok = -1);
    CHECK_ERROR(write(pipe, msg.author.c_str(), len), "", ok = -1);
    len = msg.content.length();
    CHECK_ERROR(write(pipe, &len, sizeof(int)), "", ok = -1);
    CHECK_ERROR(write(pipe, msg.content.c_str(), len), "", ok = -1);
    len = msg.channel.length();
    CHECK_ERROR(write(pipe, &len, sizeof(int)), "", ok = -1);
    CHECK_ERROR(write(pipe, msg.channel.c_str(), len), "", ok = -1);
    return ok;
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

int send_group_modify(int pipe, group_modify mod, query_id id)
{
    int ok = 0;
    CHECK_ERROR(write(pipe, &id, sizeof(query_id)), "", ok = -1);
    int n = mod.group.length();
    CHECK_ERROR(write(pipe, &n, sizeof(int)), "", ok = -1);
    CHECK_ERROR(write(pipe, mod.group.c_str(), n), "", ok = -1);
    n = mod.username.length();
    CHECK_ERROR(write(pipe, &n, sizeof(int)), "", ok = -1);
    CHECK_ERROR(write(pipe, mod.username.c_str(), n), "", ok = -1);
    return ok;
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