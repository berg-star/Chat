#include "../header/chatClient.h"
#include <iostream>
#include <thread>
#include <unistd.h>
void chatClient::errLog(const char *msg)
{
    perror(msg);
    cout << __LINE__ << " " << __func__ << " " << __FILE__;
    return;
}
void chatClient::recvMsg()
{
    char buf[sizeof(MSG)] = "";
    while (running)
    {
        bzero(buf, sizeof(buf));
        int recv_len = recv(cfd, buf, sizeof(buf), MSG_DONTWAIT);
        if (recv_len < 0)
        {
            if (errno == EAGAIN)
            {
                this_thread::sleep_for(chrono::milliseconds(500));
            }
            else
            {
                errLog("recv error");
                running = false;
                break;
            }
        }
        else if (recv_len == 0)
        {
            errLog("recv error");
            running = false;
            break;
        }else{
            MSG msg;
            msg.deSerialize(string(buf, sizeof(buf)));
            cout << msg.client_name << ":" << msg.text << endl;
        }
    }
}
void chatClient::sendMsg(int type, const string &text)
{
    MSG msg;
    msg.type = htonl(type);
    strncpy(msg.client_name, client_name.c_str(), client_name.size() + 1);
    strncpy(msg.text, text.c_str(), text.size() + 1);
    string data = msg.serialize();

    if (send(cfd, data.c_str(), data.size(), 0) < 0)
    {
        errLog("send error");
        return;
    }
    cout << "消息发送成功" << endl;
}

chatClient::chatClient(char *ip, int port, const string &name)
    : running(true),
      client_name(name)
{
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd < 0)
    {
        errLog("socket error");
        return;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port = htons(port);

    if (connect(cfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        errLog("connect error");
        return;
    }
    sendMsg(LOGIN);
}
chatClient::~chatClient()
{
    sendMsg(QUIT);

    close(cfd);
}

void chatClient::run()
{
    thread recv_thread(&chatClient::recvMsg, this);

    string text;
    while (running)
    {
        getline(cin, text);

        if (text == "quit")
        {
            cout << text << endl;
            running = false;
            break;
        }
        sendMsg(CHAT, text);
    }

    recv_thread.join();
}
