//
// Created by ironzhou on 2020/7/21.
//

#ifndef MYMQ_IPNODE_H
#define MYMQ_IPNODE_H
#include <string>
using namespace std;
class IpNode{
private:
    string ip;
    int port;

public:
    IpNode(string ip,int port):ip(ip),port(port){}
    string getIp()
    {
        return ip;
    }
    void setIp(string ip)
    {
        this->ip = ip;
    }
    int getPort()
    {
        return port;
    }
    void setPort(int port)
    {
        this->port = port;
    }
};

#endif //MYMQ_IPNODE_H
