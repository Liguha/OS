#include <zmq.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <vector>

#define CREATE 1
#define EXEC 0
#define REMOVE -1

#define CHECK_ERROR(expr) \
    do \
    { \
        int res = (expr); \
        if (res == -1) \
            return -1; \
    } while (0)

using namespace std;

int main(int argc, char* argv[])
{
    int id = atoi(argv[0]);
    string port = "tcp://*:" + to_string(id);
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    CHECK_ERROR(zmq_bind(responder, port.c_str()));

    while (true) 
    {
        zmq_msg_t msg;
        CHECK_ERROR(zmq_msg_init(&msg));
        CHECK_ERROR(zmq_msg_recv(&msg, responder, 0));
        int* data = (int*)zmq_msg_data(&msg);
        int t = *data;
        switch (t)
        {
            case CREATE:
            {
                int n = *(++data);
                string id_str = to_string(n);
                int pid = fork();
                if (pid == -1)
                    return -1;
                if (pid == 0)
                    CHECK_ERROR(execl("server", id_str.c_str(), NULL));
                else
                    CHECK_ERROR(zmq_send(responder, &pid, sizeof(int), 0));
                break;
            }

            case EXEC:
            {
                int n = *(++data);
                vector <int> v(n);
                memcpy(&v[0], (++data), n * sizeof(int));
                long long S = 0;
                for (int i = 0; i < n; i++)
                    S += v[i];
                CHECK_ERROR(zmq_send(responder, &S, sizeof(long long), 0));
                break;
            }

            case REMOVE:
            {
                zmq_send(responder, &id, sizeof(int), 0);
                zmq_close(responder);
                zmq_ctx_destroy(context);
                return 0;
            }
        }
        CHECK_ERROR(zmq_msg_close(&msg));
    }
}