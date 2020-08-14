//
// Created by ironzhou on 2020/7/26.
//
//#include "../../include/Utils/Client.h"
#include "../../include/ALL.h"

Client::Client() {}

Client::Client(int fd):clientFd(fd) {}

Client::Client(string ip, int port) : ip(ip), port(port)
{
    Connect = init(ip, port);
}

Client::Client(string ip, int port, int clientPort) : ip(ip), port(port)
{
    Connect = init(ip, port, clientPort);
}

bool Client::init(string ip, int port)
{
    // Create socket
    clientFd = socket(AF_INET, SOCK_STREAM, 0); // tcp
    if(clientFd < 0){
        cout<<"Client Create Socket Error:"<<strerror(errno)<<endl;
        exit(-1);
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    const char* ServerIp = ip.c_str();
    inet_pton(AF_INET, ServerIp, &serverAddr.sin_addr);

    // Connect
    if( connect(clientFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0 ){
        close(clientFd);
        cout<<"Client Connect Error:"<<strerror(errno)<<endl;
        //exit(-1);
        return false;
    }

    return true;
}

bool Client::init(string ip, int port, int clientPort)
{
    // Create socket
    clientFd = socket(AF_INET, SOCK_STREAM, 0); // tcp
    if(clientFd < 0){
        cout<<"Client Create Socket Error:"<<strerror(errno)<<endl;
        exit(-1);
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    const char* ServerIp = ip.c_str();
    inet_pton(AF_INET, ServerIp, &serverAddr.sin_addr);

    // Assign port to client
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(clientPort);
    clientAddr.sin_addr.s_addr = 0;
    ::bind(clientFd, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

    // Connect
    if( connect(clientFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0 ){
        close(clientFd);
        cout<<"Client Connect Error:"<<strerror(errno)<<endl;
        //exit(-1);
        return false;
    }
    return true;
}

void Client::Send(string str)
{
    //init(ip, port);
    // 写数据
    /*char* Buf = (char*)malloc(str.size());
    memset(Buf, 0, sizeof(Buf));
    memcpy(Buf, str.c_str(), str.size());
    write(clientFd, Buf, sizeof(Buf));*/
    memset(buf, 0, sizeof(buf));
    memcpy(buf, str.c_str(), str.size());
    write(clientFd, buf, sizeof(buf));

    //close(clientFd);
}

string Client::SyncSend(string str)
{
    /*char* Buf = (char*)malloc(str.size());
    memset(Buf, 0, sizeof(Buf));
    memcpy(Buf, str.c_str(), str.size());
    write(clientFd, Buf, sizeof(Buf));*/
    memset(buf, 0, sizeof(buf));
    memcpy(buf, str.c_str(), str.size());
    write(clientFd, buf, sizeof(buf));

    // 失败return "Failed"
    // 成功return Ack
    return receive();
}

string Client::SyncSend(Message msg)
{
    string str = SerializeUtil::serialize_Message(msg);
    memset(buf, 0, sizeof(buf));
    memcpy(buf, str.c_str(), str.size());
    write(clientFd, buf, sizeof(buf));

    // 失败return "Failed"
    // 成功return Ack
    return receive();
}

string Client::receive()
{
    try{
        memset(buf, 0, sizeof(buf));
        read(clientFd, buf, sizeof(buf));
        string str = buf;
        //cout<<"Client receive : "<<str<<endl;
        if(str.find("Ack") != string::npos)
            return str;
    }
    catch (std::exception e) {
        cout<<"Connection Refuse ."<<endl;
    }
    return "Failed";
}

int Client::getClientFd()
{
    return clientFd;
}

void Client::setIP(string s)
{
    ip = s;
}

void Client::setPort(int p)
{
    port = p;
}

bool Client::getConnect()
{
    return Connect;
}


