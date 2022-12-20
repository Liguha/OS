#include "unistd.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "semaphore.h"
#include "pthread.h"
#include "constants.hpp"
#include "instructions.hpp"
#include <iostream>
#include <string>

using namespace std;

void* get_thd(void* ptr)
{
    user_info* info = (user_info*)ptr;
    string str_id = to_string(info->user_id);
    int pipe = open((str_id + get_postfix).c_str(), O_RDWR);
    while (true)
    {
        query_id id;
        read(pipe, &id, sizeof(id));
        switch (id)
        {
            case LOGIN_OK:
            {
                cout << "Logged-in" << endl;
                break;
            }

            case LOGIN_ERR:
            {
                info->username = "";
                cerr << "Error: user already logged-in" << endl;
                break;
            }

            case SEND_ERR:
            {
                cerr << "Error: can't find receiver" << endl;
                break;
            }

            case GET_PRIVATE:
            {
                message msg = receive_msg(pipe);
                cout << "Private message from " << msg.author << ": " << msg.content << endl;
                break;
            }

            case GET_GROUP:
            {
                message msg = receive_msg(pipe);
                cout << "Message in chat " << msg.channel << " from " << msg.author << ": " << msg.content << endl;
                break;
            }

            case CREATE_G_ERR:
            {
                cerr << "Error: can't create such group" << endl;
                break;
            }

            case ADD_G_OK:
            {
                int n;
                read(pipe, &n, sizeof(int));
                char* str = (char*)calloc(n, sizeof(char));
                read(pipe, str, n);
                cout << "Welcome in chat " << str << endl;
                free(str);
                break;
            }

            case ADD_G_ERR:
            {
                cerr << "Error: can't add this user in this group" << endl;
                break;
            }
        }
    }
    return NULL;
}

int main()
{
    user_info info;
    info.username = "";
    info.user_id = getpid();
    pthread_t thd;
    int data = open(data_pipe.c_str(), O_RDWR);
    sem_t* sem = sem_open(sem_name.c_str(), O_RDWR);

    string str_id = to_string(info.user_id);
    string str_send, str_get;
    str_send = str_id + send_postfix;
    str_get = str_id + get_postfix;
    unlink(str_send.c_str());
    unlink(str_get.c_str());
    mkfifo(str_send.c_str(), S_IREAD | S_IWRITE);
    mkfifo(str_get.c_str(), S_IREAD | S_IWRITE);
    int pipe = open(str_send.c_str(), O_RDWR);;
    while (true)
    {
        string command;
        query_id id;
        cin >> command;

        if (command == "login")
        {
            id = LOGIN;
            string name;
            cin >> name;
            if (info.username != "")
            {
                cerr << "Error: you are already logged-in" << endl;
                continue;;
            }
            if (name == "")
            {
                cout << "Please, enter correct name" << endl;
                continue;
            }
            info.username = name;
            pthread_create(&thd, NULL, get_thd, &info);
            pthread_detach(thd);
            sem_wait(sem);
            write(data, &id, sizeof(query_id));
            write(data, &info.user_id, sizeof(int));
            int len = info.username.length();
            write(data, &len, sizeof(int));
            write(data, info.username.c_str(), len);
            sem_post(sem);
        }

        if (command == "send")
        {
            if (info.username == "")
            {
                cout << "Please, login in the system" << endl;
                continue;
            }
            string type, dst, str;
            cin >> type >> dst;
            if (type != "private" && type != "chat")
            {
                cerr << "Error: unknown message type " << type << endl;
                continue;
            }
            getline(cin, str);
            message msg;
            msg.author = info.username;
            msg.content = str;
            msg.channel = dst;
            if (type == "private")
                id = SEND_PRIVATE;
            if (type == "chat")
                id = SEND_GROUP;
            send_message(pipe, msg, id);
        }

        if (command == "chat")
        {
            string act, group;
            cin >> act >> group;
            if (act != "create" && act != "add")
            {
                cerr << "Error: unknown chat action" << endl;
                continue;
            }
            group_modify mod;
            mod.group = group;
            if (act == "create")
            {
                mod.username = info.username;
                id = CREATE_GROUP;
            }
            if (act == "add")
            {
                cin >> mod.username;
                id = ADD_TO_GROUP;
            }
            send_group_modify(pipe, mod, id);
        }
    }
}