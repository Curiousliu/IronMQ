//
// Created by ironzhou on 2020/8/7.
//
#include "../../include/ALL.h"

TopicRegister::TopicRegister() {}

TopicRegister::TopicRegister(Topic topic, IpNode i)
:topic(topic), ipNode(i)
{
    Register();
}

void TopicRegister::Register()
{
    cout<<"正在为Topic "<<topic.getTopicName()<<" 申请队列..."<<endl;
    try{
        Client client(ipNode.getIp(), ipNode.getPort());
        int fd = client.getClientFd();
        if(fd < 0)
        {
            cout<<"连接失败"<<endl;
        }
        Message msg( 1, MessageType::REQUEST_QUEUE, topic,"Request Queue");
        string ret = client.SyncSend(msg);
        if(ret.find("Ack") != string::npos)
        {
            cout<<"申请成功 : "<<ret<<endl;
        }
        else
        {
            cout<<"申请失败, ret : "<<ret<<endl;
        }
        close(fd);
    }
    catch(std::exception e)
    {
        cout<<"Connection Refuse. TOPIC REGISTER"<<endl;
    }

}


