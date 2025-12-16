#include"../head/chatServer.h"
#include<iostream>
int main(int argc,char *argv[]){
    if(argc < 3){
        cout << "请正确输入ip和端口号" << endl;
    }
    chatServer server(argv[1],atoi(argv[2]),10);
    server.run();
    return 0;
}