//
// Created by ironzhou on 2020/8/5.
//
#include "../../include/ALL.h"

int main(){
    Broker broker(serverPort);
    bool isRecovery = false;
    if(isRecovery)
    {
        broker = broker.recover();
    }
    IpNode slave("127.0.0.1", slavePort);
    set<IpNode> slaves;
    slaves.insert(slave);
    broker.setSlave(slaves);
    /*broker.setPushTime(1000);
    broker.setRetryTime(16);
    broker.setSyncTime(1000);*/
    broker.setStoreTime(10000); // 10s一次
    broker.setStartPersistence(true);
    broker.setPushMode(true);
    broker.setPushTime(10);
    //std::system("pause");
    pause();
    return 0;
}