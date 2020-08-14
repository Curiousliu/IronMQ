//
// Created by ironzhou on 2020/8/6.
//
#include "../../include/ALL.h"

int main(){
    IpNode ipNode("127.0.0.1",serverPort);

    Consumer consumer1(1),consumer2(2),consumer3(3),consumer4(4);
    consumer1.Register(ipNode, consumerPort1);
    consumer1.listening();
    consumer2.Register(ipNode, consumerPort2);
    consumer2.listening();
    consumer3.Register(ipNode, consumerPort3);
    consumer3.listening();
    consumer4.Register(ipNode, consumerPort4);
    consumer4.listening();


    //cout<<"End Time : "<<consumer4.getTend()<<endl;


    pause();
    return 0;
}
