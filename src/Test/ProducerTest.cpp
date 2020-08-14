//
// Created by ironzhou on 2020/8/7.
//
#include "../../include/ALL.h"
int Producer::reTry_Time = 16;
IpNode broker("127.0.0.1",serverPort);
Client Producer::client(broker.getIp(), broker.getPort());
int main(){

    Topic topic1("order"), topic2("delivery");

    boost::thread_group threadGroup;
    SequenceUtil sequenceUtil;

    int m = 0, n = 0;


    time_t t_start1 = time(0);

    for(int i = 0 ; i < TOTAL_MESSAGE_NUM; i++)
    {
        int num = sequenceUtil.getSequence();
        string s = "order";
        s += " "+to_string(m++);
        Message msg(num,MessageType::REPLY_EXPECTED,topic1,s);
        msg.setPriority(0);
        //Producer::Send(msg, broker);
        Producer::Send(msg);

        num = sequenceUtil.getSequence();
        string s2 = "delivery";
        s2 += " "+to_string(n++);
        Message msg2(num,MessageType::REPLY_EXPECTED,topic2,s2);
        msg2.setPriority(0);
        Producer::Send(msg2);
        //Producer::Send(msg2, broker);
    }

    time_t t_end1 = time(0);

    cout<<"Diff time1 : "<<difftime(t_end1, t_start1)<<endl;


    //cout<<"Start Time (s) : "<<t_start<<endl;






    //cout<<Producer::Send(new Message(),broker);

    cout<<"producer test"<<endl;
    threadGroup.join_all();
    pause();
    return 0;
}
