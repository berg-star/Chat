#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<string>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<vector>
#include<mutex>
#include<thread>
#include<functional>
#include<queue>
#include<condition_variable>
using namespace std;

#define N 128
#define LOGIN 1
#define CHAT 2
#define QUIT 3
class chatServer{
    public:
    struct MSG{
        int type;
        char client_name[20];
        char text[N];

        string serialize() const{
            string data;
            data.append(reinterpret_cast<const char *>(&type),sizeof(type));
            data.append(client_name,sizeof(client_name));
            data.append(text,sizeof(text));
            return data;
        }

        void deSerialize(const string &data){
            size_t offset = 0;
            memcpy(&type,data.c_str() + offset,sizeof(type));
            offset += sizeof(type);
            memcpy(client_name,data.c_str() + offset,sizeof(client_name));
            offset += sizeof(client_name);
            memcpy(text,data.c_str() + offset,sizeof(text));
        }
        };
        struct Client{
            int fd;
            struct sockaddr_in cin;
        };

        chatServer(const char *ip,int port,size_t thread_pool_size);
        ~chatServer();
        void run();
        void handleClient(int client_fd,struct sockaddr_in cin);
        void broadcast(const MSG &msg,int exclude_fd = -1);
    private:
        int sfd;
        bool stop;
        condition_variable task_cv;
        vector<Client>clients;
        mutex client_mutex;
        mutex task_mutex;
        vector<thread>works;
        queue<function<void()>>tasks;

        void startThreadPool(size_t numThreads);
        void errLog(const char *msg);
        void addTask(function<void()>task);
};

#endif