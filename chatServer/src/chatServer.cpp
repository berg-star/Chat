#include"../head/chatServer.h"
#include<iostream>
#include<unistd.h>
#define ERR_LOG(msg)                                            \
    do                                                          \
    {                                                           \
        perror(msg);                                            \
        cout << __LINE__ << " " << __func__ << " " << __FILE__; \
    } while (0);

chatServer::chatServer(const char *ip,int port,size_t thread_pool_size)
:stop(false)
{
    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd < 0){
        errLog("socket error");
        return;
    }

    int reuse = 1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) < 0){
        errLog("setsockopt error");
        return;
    }

    struct sockaddr_in sin;
    sin.sin_port = htons(port);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);

    if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin)) < 0){
        errLog("bind error");
        return;
    }

    if(listen(sfd,20) < 0){
        errLog("listen error");
        return;
    }
    startThreadPool(thread_pool_size);
}
chatServer::~chatServer(){
    {
        unique_lock<mutex>lock(task_mutex);
        stop = true;
    }
    task_cv.notify_all();
    for(auto &work:works){
        work.join();
    }
}
void chatServer::run(){

    while (true)
    {
        struct sockaddr_in cin;
        socklen_t len = sizeof(cin);
        int client_fd = accept(sfd,(struct sockaddr*)&cin,&len);
        if(client_fd < 0){
            errLog("accept error");
            continue;
        }
        addTask([this,client_fd,cin]{
            this->handleClient(client_fd,cin);
        });
    }

}

void chatServer::handleClient(int client_fd,struct sockaddr_in cin){
    MSG msg;
    char buf[sizeof(msg)];
    while(true)
    {   
        int recv_len = recv(client_fd,buf,sizeof(buf),0);
        if(recv_len <= 0){
            vector<Client>::iterator it = clients.begin();
            while(it != clients.end()){
                if(it->fd == client_fd){
                    clients.erase(it);
                    break;
                }
                ++it;
            }
            close(client_fd);
            break; 
        }
        msg.deSerialize(string(buf,recv_len));

        switch(ntohl(msg.type))
        {
            case LOGIN:
                Client client;
                client.fd = client_fd;
                client.cin = cin;
                {
                    unique_lock<mutex>lock(client_mutex);
                    clients.push_back(client);
                    sprintf(msg.text,"------%s登陆成功------",msg.client_name);
                    printf("%s 登陆成功 client_fd = %d\n",msg.client_name,client_fd);
                    broadcast(msg);
                }
                break;
            case CHAT:
                {
                    unique_lock<mutex>lock(client_mutex);
                    broadcast(msg,client_fd);
                }
                break;
            case QUIT:
                {
                    unique_lock<mutex>lock(client_mutex);
                    vector<Client>::iterator it = clients.begin();
                    while(it != clients.end())
                    {
                        if(it->fd == client_fd)
                        {
                            clients.erase(it);
                            break;
                        }
                        ++it;
                    }
                    sprintf(msg.text,"-----%s退出聊天室-----",msg.client_name);
                    broadcast(msg);
                    close(client_fd);
                    break;
                }
            default:
                errLog("消息有误");
                break;
        }

    }

}

void chatServer::broadcast(const MSG &msg,int exclude_fd){
    string data = msg.serialize();
    for(auto &client:clients){
        if(client.fd != exclude_fd){
            cout << exclude_fd << " " << client.fd << endl;
            if(send(client.fd,data.c_str(),data.size(),0) < 0){
                errLog("send msg error");
                return;
            }
        }
    }

}

void chatServer::startThreadPool(size_t numThreads){
    for (int i = 0; i < numThreads; i++)
    {
        works.emplace_back([this]
        {
            while(true)
            {
                function<void()>task;
                {
                    unique_lock<mutex>lock(task_mutex);
                    task_cv.wait(lock,[this]{
                        return stop || !tasks.empty();
                    });
                    if(stop && tasks.empty()){
                        return;
                    }
                    task = move(tasks.front());
                    tasks.pop();

                }
                task();
            }

        });
    }
    
}

void chatServer::errLog(const char *msg){
     perror(msg);                                            
     cout << __LINE__ << " " << __func__ << " " << __FILE__;
     return;
}

void chatServer::addTask(function<void()>task){
    {
        unique_lock<mutex>lock(task_mutex);
        tasks.push(task);
    }
        task_cv.notify_one();
}
