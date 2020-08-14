//
// Created by ironzhou on 2020/7/24.
//

//#include "../../include/Common/Topic.h"

#include "../../include/ALL.h"

bool Topic::operator<(const Topic &t) const
{
    return topicName < t.topicName;
}


Topic::Topic(){}

Topic::Topic(string s):topicName(s){}

Topic::Topic(string s, int queueNum) :topicName(s),queueNum(queueNum){}

Topic::Topic(string s, int queueNum, set<IpNode> consumer_addr)
        :topicName(s), queueNum(queueNum), consumer_address(consumer_addr){}

/*Topic::Topic(const Topic &topic)
{
    queueNum = topic.queueNum;
    topicName = topic.topicName;
    consumer_address = topic.consumer_address;
}*/

string Topic::getTopicName()
{
    return topicName;
}


set<IpNode> Topic::getConsumers()
{
    return consumer_address;
}

void Topic::addConsumer(IpNode i)
{
    consumer_address.insert(i);
}

void Topic::deleteConsumer(IpNode i)
{
    if(consumer_address.count(i))
        consumer_address.erase(i);
}


int Topic::getQueueNum()
{
    return queueNum;
}