#include"../header/chatClient.h"
#include<iostream>
int main(int argc,char *argv[]){

    if(argc < 4){
        cout << "请输入正确的参数" << endl;
    }
    printf("name = %s",argv[3]);
    chatClient client(argv[1],atoi(argv[2]), argv[3]);
    client.run();
    return 0;
}