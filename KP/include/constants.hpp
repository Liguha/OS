#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>

using namespace std;

enum query_id 
{ 
    LOGIN, 
    LOGIN_OK, 
    LOGIN_ERR, 
    SEND_PRIVATE, 
    SEND_ERR, 
    GET_PRIVATE, 
    SEND_GROUP, 
    GET_GROUP, 
    CREATE_GROUP,
    CREATE_G_OK,
    CREATE_G_ERR,
    ADD_TO_GROUP,
    ADD_G_OK,
    ADD_G_ERR,
    LOGOUT,
    EXIT
};
const string sem_name = "data_semaphore";
const string data_pipe = "login_data";
const string send_postfix = "_send";
const string get_postfix = "_get";

#endif