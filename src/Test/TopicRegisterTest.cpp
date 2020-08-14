//
// Created by ironzhou on 2020/8/7.
//
#include "../../include/ALL.h"

int main()
{
    IpNode broker("127.0.0.1",serverPort),
    consumer1("127.0.0.1", consumerPort1), consumer2("127.0.0.1", consumerPort2),
    consumer3("127.0.0.1", consumerPort3), consumer4("127.0.0.1", consumerPort4);

    set<IpNode> consumers_for_topic1, consumers_for_topic2;
    consumers_for_topic1.clear();consumers_for_topic2.clear();
    consumers_for_topic1.insert(consumer1);
    consumers_for_topic1.insert(consumer2);
    consumers_for_topic2.insert(consumer3);
    consumers_for_topic2.insert(consumer4);
    Topic topic1("order",32,consumers_for_topic1), topic2("delivery",32,consumers_for_topic2);
    TopicRegister topicRegister1(topic1, broker), topicRegister2(topic2, broker);



    pause();
    return 0;
}
