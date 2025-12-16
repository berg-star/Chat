#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include<string>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
using namespace std;

#define N 128
#define LOGIN 1
#define CHAT 2
#define QUIT 3

class chatClient{
    public:
    struct MSG
    {
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

    chatClient(char *ip,int port,const string &name);
    ~chatClient();
    void run();
    
    private:
        bool running;
        int cfd;
        string client_name;
        void errLog(const char* msg);
        void recvMsg();
        void sendMsg(int type,const string &text = "");
};
#endif