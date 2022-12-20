#include "unistd.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "semaphore.h"
#include "pthread.h"
#include "constants.hpp"
#include "instructions.hpp"
#include <iostream>
#include <string>
#include <set>
#include <map>

using namespace std;

void* user_thd(void*);

struct user
{
    user_info info;
    int pipe_to;
    pthread_t thd;

    user(string login = "", int id = 0)
    {
        info.username = login;
        info.user_id = id;
        pipe_to = open((to_string(id) + get_postfix).c_str(), O_RDWR);
        cout << to_string(id) + get_postfix << " , handle: " << pipe_to << endl;
        pthread_create(&thd, NULL, user_thd, this);
        pthread_detach(thd);
    }

    ~user()
    {
        close(pipe_to);
    }
};

struct group
{
    string name;
    set <user*> members;
};

map <int, user*> users_id;
map <string, user*> users;
map <string, group*> groups;

void* user_thd(void* ptr)
{
    user* usr = (user*)ptr;
    int pipe = open((to_string(usr->info.user_id) + send_postfix).c_str(), O_RDWR);
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
                    write(usr->pipe_to, &ans, sizeof(query_id));
                    break;
                }
                int pipe_to = users[msg.channel]->pipe_to;
                send_message(pipe_to, msg, GET_PRIVATE);
                break;
            }

            case SEND_GROUP:
            {
                message msg = receive_msg(pipe);
                if (groups.count(msg.channel) == 0)
                {
                    query_id ans = SEND_ERR;
                    write(usr->pipe_to, &ans, sizeof(query_id));
                    break;
                }
                set <user*>& g_users = groups[msg.channel]->members;
                if (g_users.count(usr) == 0)
                {
                    query_id ans = SEND_ERR;
                    write(usr->pipe_to, &ans, sizeof(query_id));
                    break;
                }
                for (auto it = g_users.begin(); it != g_users.end(); it++)
                {
                    if ((*it)->info.username == msg.author)
                        continue;
                    int pipe_to = (*it)->pipe_to;
                    send_message(pipe_to, msg, GET_GROUP);
                }
                break;
            }

            case CREATE_GROUP:
            {
                group_modify mod = receive_group_modify(pipe);
                if (groups.count(mod.group) != 0)
                {
                    query_id ans = CREATE_G_ERR;
                    write(usr->pipe_to, &ans, sizeof(query_id));
                    break;
                }
                groups[mod.group] = new group();
                groups[mod.group]->name = mod.group;
                groups[mod.group]->members.insert(usr);
                break;
            }

            case ADD_TO_GROUP:
            {
                group_modify mod = receive_group_modify(pipe);
                if (groups.count(mod.group) == 0 || users.count(mod.username) == 0)
                {
                    query_id ans = ADD_G_ERR;
                    write(usr->pipe_to, &ans, sizeof(query_id));
                    break;
                }
                set <user*>& g_users = groups[mod.group]->members;
                if (g_users.count(usr) == 0 || g_users.count(users[mod.username]) != 0)
                {
                    query_id ans = ADD_G_ERR;
                    write(usr->pipe_to, &ans, sizeof(query_id));
                    break;
                }
                g_users.insert(users[mod.username]);
                query_id ans = ADD_G_OK;
                write(users[mod.username]->pipe_to, &ans, sizeof(query_id));
                int n = mod.group.length();
                write(users[mod.username]->pipe_to, &n, sizeof(int));
                write(users[mod.username]->pipe_to, mod.group.c_str(), n);
                break;
            }
        }
    }
    return NULL;
}

int main()
{
    unlink(data_pipe.c_str());
    mkfifo(data_pipe.c_str(), S_IREAD | S_IWRITE);
    sem_unlink(sem_name.c_str());
    int data = open(data_pipe.c_str(), O_RDWR);
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
                read(data, &user_id, sizeof(int));
                int len;
                read(data, &len, sizeof(int));
                char* str = (char*)calloc(len, sizeof(char));
                read(data, str, len);
                string name = str;
                free(str);
                query_id ans;
                if (users_id.count(user_id) == 0)
                    users_id[user_id] = new user(name, user_id);
                if (users.count(name) == 0)
                {
                    users[name] = users_id[user_id];
                    ans = LOGIN_OK;
                }
                else
                {
                    users_id[user_id]->info.username = "";
                    ans = LOGIN_ERR;
                }
                write(users_id[user_id]->pipe_to, &ans, sizeof(query_id));
                break;
            }
        }
    }
}