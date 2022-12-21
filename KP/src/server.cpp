#include "unistd.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "semaphore.h"
#include "pthread.h"

#include "constants.hpp"
#include "instructions.hpp"
#include "check_err.hpp"

#include <iostream>
#include <string>
#include <set>
#include <map>

using namespace std;

void* user_thd(void*);

struct user
{
    user_info info;
    int pipe_from, pipe_to;
    pthread_t thd;

    user(string login = "", int id = 0)
    {
        info.username = login;
        info.user_id = id;
        pipe_to = open((to_string(id) + get_postfix).c_str(), O_RDWR);
        pipe_from = open((to_string(id) + send_postfix).c_str(), O_RDWR);
        CHECK_ERROR(min(pipe_to, pipe_from), "Error: pipe of " << id << '\n', return);
        if (pthread_create(&thd, NULL, user_thd, this) != 0)
        {
            cerr << "Error: creating thread (id = " << id << ")\n";
            return;
        }
        if (pthread_detach(thd) != 0)
            cerr << "Error: detaching thread (id = " << id << ")\n";
    }

    ~user()
    {
        close(pipe_from);
        close(pipe_to);
    }
};

struct group
{
    string name;
    set <string> members;
};

map <int, user*> users_id;
map <string, user*> users;
map <string, group*> groups;

void* user_thd(void* ptr)
{
    user* usr = (user*)ptr;
    int pipe = usr->pipe_from;
    while (true)
    {
        query_id id;
        read(pipe, &id, sizeof(query_id));
        switch (id)
        {
            case SEND_PRIVATE:
            {
                message msg = receive_msg(pipe);
                if (users.count(msg.channel) == 0)
                {
                    query_id ans = SEND_ERR;
                    CHECK_ERROR(write(usr->pipe_to, &ans, sizeof(query_id)), "Error: answering",);
                    break;
                }
                int pipe_to = users[msg.channel]->pipe_to;
                CHECK_ERROR(send_message(pipe_to, msg, GET_PRIVATE), "Error: message error\n",);
                break;
            }

            case SEND_GROUP:
            {
                message msg = receive_msg(pipe);
                if (groups.count(msg.channel) == 0)
                {
                    query_id ans = SEND_ERR;
                    CHECK_ERROR(write(usr->pipe_to, &ans, sizeof(query_id)), "Error: answering",);
                    break;
                }
                set <string>& g_users = groups[msg.channel]->members;
                if (g_users.count(usr->info.username) == 0)
                {
                    query_id ans = SEND_ERR;
                    CHECK_ERROR(write(usr->pipe_to, &ans, sizeof(query_id)), "Error: answering",);
                    break;
                }
                for (auto it = g_users.begin(); it != g_users.end(); it++)
                {
                    if (*it == msg.author || users.count(*it) == 0)
                        continue;
                    int pipe_to = users[*it]->pipe_to;
                    CHECK_ERROR(send_message(pipe_to, msg, GET_GROUP), "Error: message error\n",);
                }
                break;
            }

            case CREATE_GROUP:
            {
                group_modify mod = receive_group_modify(pipe);
                if (groups.count(mod.group) != 0)
                {
                    query_id ans = CREATE_G_ERR;
                    CHECK_ERROR(write(usr->pipe_to, &ans, sizeof(query_id)), "Error: message error\n",);
                    break;
                }
                groups[mod.group] = new group();
                groups[mod.group]->name = mod.group;
                groups[mod.group]->members.insert(usr->info.username);
                break;
            }

            case ADD_TO_GROUP:
            {
                group_modify mod = receive_group_modify(pipe);
                if (groups.count(mod.group) == 0 || users.count(mod.username) == 0)
                {
                    query_id ans = ADD_G_ERR;
                    CHECK_ERROR(write(usr->pipe_to, &ans, sizeof(query_id)), "Error: message error\n",);
                    break;
                }
                set <string>& g_users = groups[mod.group]->members;
                if (g_users.count(usr->info.username) == 0 || g_users.count(mod.username) != 0)
                {
                    query_id ans = ADD_G_ERR;
                    CHECK_ERROR(write(usr->pipe_to, &ans, sizeof(query_id)), "Error: message error\n",);
                    break;
                }
                g_users.insert(mod.username);
                query_id ans = ADD_G_OK;
                CHECK_ERROR(write(users[mod.username]->pipe_to, &ans, sizeof(query_id)), "Error: adding in group\n", break);
                int n = mod.group.length();
                CHECK_ERROR(write(users[mod.username]->pipe_to, &n, sizeof(int)), "Error: adding in group\n", break);
                CHECK_ERROR(write(users[mod.username]->pipe_to, mod.group.c_str(), n), "Error: adding in group\n", break);
                break;
            }

            case LOGOUT:
            {
                string login = usr->info.username;
                usr->info.username = "";
                users.erase(login);
                break;
            }

            case EXIT:
            {
                write(usr->pipe_to, &id, sizeof(query_id));
                delete usr;
                return NULL;
            }
        }
    }
    return NULL;
}

int main()
{
    unlink(data_pipe.c_str());
    CHECK_ERROR(mkfifo(data_pipe.c_str(), S_IREAD | S_IWRITE), "Error: creating data pipe\n", return -1);
    sem_unlink(sem_name.c_str());
    int data = open(data_pipe.c_str(), O_RDWR);
    CHECK_ERROR(data, "Error: opening data pipe\n", return -1);
    sem_t* sem = sem_open(sem_name.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
    while (true)
    {
        query_id id;
        read(data, &id, sizeof(query_id));
        switch (id)
        {
            case LOGIN:
            {
                int user_id;
                CHECK_ERROR(read(data, &user_id, sizeof(int)), "Error: reading user data\n", break);
                int len;
                CHECK_ERROR(read(data, &len, sizeof(int)), "Error: reading user data\n", break);
                char* str = (char*)calloc(len, sizeof(char));
                CHECK_ERROR(read(data, str, len), "Error: reading user data\n", break);
                string name = str;
                free(str);
                query_id ans;
                if (users_id.count(user_id) == 0)
                    users_id[user_id] = new user(name, user_id);
                if (users.count(name) == 0)
                {
                    users_id[user_id]->info.username = name;
                    users[name] = users_id[user_id];
                    ans = LOGIN_OK;
                }
                else
                {
                    users_id[user_id]->info.username = "";
                    ans = LOGIN_ERR;
                }
                CHECK_ERROR(write(users_id[user_id]->pipe_to, &ans, sizeof(query_id)), "Error: answering\n",);
                break;
            }

            case EXIT:
            {
                int usr_id;
                CHECK_ERROR(read(data, &usr_id, sizeof(int)), "Error: reading user id\n", break);
                if (users_id.count(usr_id) != 0)
                {
                    user* usr = users_id[usr_id];
                    query_id ans = EXIT;
                    write(usr->pipe_from, &ans, sizeof(query_id));
                }
                else
                {
                    int pipe_to = open((to_string(usr_id) + get_postfix).c_str(), O_RDWR);
                    write(pipe_to, &id, sizeof(query_id));
                    close(pipe_to);
                }
                break;
            }
        }
    }
}