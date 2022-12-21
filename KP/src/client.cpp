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

using namespace std;

bool ACTIVE_THD = true;
bool ACTIVE_MAIN = true;

void* get_thd(void* ptr)
{
    user_info* info = (user_info*)ptr;
    string str_id = to_string(info->user_id);
    int pipe = open((str_id + get_postfix).c_str(), O_RDWR);
    CHECK_ERROR(pipe, "Error: can't open pipe" << endl, ACTIVE_THD = false);
    while (ACTIVE_THD)
    {
        query_id id;
        CHECK_ERROR(read(pipe, &id, sizeof(id)), "Error: pipe read error" << endl, ACTIVE_THD = false);
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

            case CREATE_G_OK:
            {
                cout << "Success create chat" << endl;
                break;
            }

            case CREATE_G_ERR:
            {
                cerr << "Error: can't create such chat" << endl;
                break;
            }

            case ADD_G_OK:
            {
                int n;
                CHECK_ERROR(read(pipe, &n, sizeof(int)), "Error: pipe read error" << endl, ACTIVE_THD = false);
                char* str = (char*)calloc(n, sizeof(char));
                CHECK_ERROR(read(pipe, str, n), "Error: pipe read error" << endl, ACTIVE_THD = false);
                cout << "Welcome in chat " << str << endl;
                free(str);
                break;
            }

            case ADD_G_ERR:
            {
                cerr << "Error: can't add this user in this chat" << endl;
                break;
            }

            case EXIT:
            {
                close(pipe);
                unlink((to_string(info->user_id) + get_postfix).c_str());
                unlink((to_string(info->user_id) + send_postfix).c_str());
                ACTIVE_MAIN = false;
                return NULL;
            }
        }
    }
    close(pipe);
    unlink((to_string(info->user_id) + get_postfix).c_str());
    unlink((to_string(info->user_id) + send_postfix).c_str());
    ACTIVE_MAIN = false;
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
    CHECK_ERROR(mkfifo(str_send.c_str(), S_IREAD | S_IWRITE), "Error: mkfifo error\n", return -1);
    CHECK_ERROR(mkfifo(str_get.c_str(), S_IREAD | S_IWRITE), "Error: mkfifo error\n", return -1);
    CHECK_ERROR_PTHREAD(pthread_create(&thd, NULL, get_thd, &info), "Error: error of creating thread\n");
    CHECK_ERROR_PTHREAD(pthread_detach(thd), "Error: error of detach thread\n");
    int pipe = open(str_send.c_str(), O_RDWR);
    CHECK_ERROR(pipe, "Error: can't open pipe\n", return -1);

    while (ACTIVE_MAIN)
    {
        string command;
        query_id id;
        cin >> command;

        if (!ACTIVE_MAIN)
            return -1;

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
            sem_wait(sem);
            CHECK_ERROR(write(data, &id, sizeof(query_id)), "Error: data pipe writing\n", return -1);
            CHECK_ERROR(write(data, &info.user_id, sizeof(int)), "Error: data pipe writing\n", return -1);
            int len = info.username.length();
            CHECK_ERROR(write(data, &len, sizeof(int)), "Error: data pipe writing\n", return -1);
            CHECK_ERROR(write(data, info.username.c_str(), len), "Error: data pipe writing\n", return -1);
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
            CHECK_ERROR(send_message(pipe, msg, id), "Error: sending message\n", return -1);
        }

        if (command == "chat")
        {
            if (info.username == "")
            {
                cout << "Please, login in the system" << endl;
                continue;
            }
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
            CHECK_ERROR(send_group_modify(pipe, mod, id), "Error: modify chat\n", return -1);
        }

        if (command == "logout")
        {
            if (info.username == "")
            {
                cout << "Please, login in the system" << endl;
                continue;
            }
            id = LOGOUT;
            CHECK_ERROR(write(pipe, &id, sizeof(query_id)), "Error: pipe writing\n", return -1);
            info.username = "";
            cout << "Logged-out" << endl;
        }

        if (command == "exit")
        {
            if (info.username != "")
            {
                id = LOGOUT;
                write(pipe, &id, sizeof(query_id));
            }
            id = EXIT;
            sem_wait(sem);
            write(data, &id, sizeof(query_id));
            write(data, &info.user_id, sizeof(int));
            sem_post(sem);
            close(pipe);
            close(data);
            sem_close(sem);
            while (ACTIVE_MAIN)
                continue;
            return 0;
        }
    }
    return -1;
}