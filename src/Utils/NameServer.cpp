//
// Created by ironzhou on 2020/8/12.
//
#include "../../include/ALL.h"

NameServer::NameServer(int port)
{
    RequestProcessor RequestProcessor;
    ResponseProcessor NameServerResponseProcessor(2);
    Slave slave1;
    Broker broker1;

    cout<<"NameServer 已启动, port : "<<port<<endl;
    auto _server = [&]()
    {
        try{
            new Server(port, RequestProcessor, NameServerResponseProcessor, broker1, slave1, this);
        }
        catch (std::exception exception) {
            cout<<"NameServer didn't launch"<<endl;
        }
    };
    auto _checkStat = [&]()
    {
        while(true)
        {
            try{
                boost::this_thread::sleep_for(boost::chrono::milliseconds(60000)); // 每分钟检查一次
            }
            catch (std::exception e) {
                cerr<<e.what();
            }

            for(auto it : Brokers)
            {
                time_t now = time(0);
                if(difftime(now, it.second.first) > 120) // 算上网络延迟 两分钟没收到就算broker宕机
                {
                    it.second.second = false; // 宕机
                }
                BrokerClients.erase(it.first); // 有必要删除与该broker对应的文件描述符?
            }
        }
    };



    thread thrd(_server);
    thread thrd2(_checkStat);
}
