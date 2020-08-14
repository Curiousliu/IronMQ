//
// Created by ironzhou on 2020/8/8.
//

#include "../../include/ALL.h"

void Producer::setRetryTime(int num)
{
    reTry_Time = num;
}

void Producer::Send(Message msg, IpNode ipNode)
{
    try{
        Client client(ipNode.getIp(), ipNode.getPort());
        for(int i = 0 ; i < reTry_Time ; i++)
        {
            string result = client.SyncSend(msg);
            if(result.find("Ack") != string::npos)
            {
                cout<<"Message "<<msg.getNum()<<" Sending Succeed!"<<endl;
                cout << "Receive : " <<result << endl;
                return;
            }
            if(result.find("Failed") != string::npos)
            {
                cout<<"Message "<<msg.getNum()<<" Sending Failed! Receive : "<<result<<". Retrying the "<<i<<" th time"<<endl;
            }
        }
    }
    catch (std::exception e) {
        cout<<"Broker未上线"<<endl;
    }
    cout<< "Failed Sending Message Num : " << msg.getNum() << endl;
}

void Producer::Send(Message msg)
{
    for(int i = 0 ; i < reTry_Time ; i++)
    {
        string result = client.SyncSend(msg);
        if(result.find("Ack") != string::npos)
        {
            cout<<"Message "<<msg.getNum()<<" Sending Succeed!"<<endl;
            cout << "Receive : " <<result << endl;
            return;
        }
        if(result.find("Failed") != string::npos)
        {
            cout<<"Message "<<msg.getNum()<<" Sending Failed! Receive : "<<result<<". Retrying the "<<i<<" th time"<<endl;
        }
    }
}
