//
// Created by ironzhou on 2020/7/24.
//
//#include "../../include/Broker/Slave.h"
#include "../../include/ALL.h"
using namespace std;
using namespace boost;
Slave::Slave() {}

Slave::Slave(int port/*slave监听端口*/)
{
    cout << "Slave已启动"<<endl;

    RequestProcessor RequestProcessor;
    ResponseProcessor SlaveResponseProcessor(1);
    Slave *slave = this;
    broker = new Broker();

    auto f = [&](){
        try{
            new Server(port, RequestProcessor, SlaveResponseProcessor, *broker, *slave);
        }
        catch (std::exception e) {
            cerr << "Slave"<<e.what();
        }

    };
    thread thrd(f);
}
