//
// Created by ironzhou on 2020/7/26.
//
//#include "../../include/Broker/Filter.h"
#include "../../include/ALL.h"
Filter::Filter() {}

Filter::Filter(map<Topic, set<IpNode> > Map)
:topicConsumerMap(Map){}

map<Topic, set<IpNode>> Filter::getTopicConsumerMap()
{
    return topicConsumerMap;
}

map<IpNode, set<Message>> Filter::classification(map<IpNode, int> clients, list<Message> l)
{
    map<IpNode, set<Message> > Map;
    for(auto it : l)
    {
        set<IpNode> consumers = topicConsumerMap[it.getTopic()];
        for(auto that : consumers)
        {
            Map[that].insert(it);
        }
    }
    return Map;
}
/*
Filter::Filter(list<IpNode> index):index(index){}

map<IpNode, list<Message> > Filter::filter(list<Message> l)
{
    // 将Message按照分发地址分类
    map<IpNode, list<Message> > Map;
    // 初始化
    for(IpNode address : index)
    {
        if(Map.count(address) == 0)
        {
            list<Message> *temp_l = new list<Message>();
            Map.insert(make_pair(address, *temp_l));
        }
    }
    //遍历消息, 将每条message分类
    for(Message it : l)
    {
        list<IpNode> consumer_address = it.getTopic().getConsumer();
        for(IpNode that : consumer_address)
        {
            if(Map.find(that) != Map.end())
            {
                list<Message> &lm = Map[that];
                lm.push_back(it);
            }
        }
    }
    return Map;
}


/*int main(){
    IpNode n1("127.0.0.1",80), n2("127.0.0.1",100),n3("192.168.1.1",80);
    list<IpNode> index;
    index.push_back(n1); index.push_back(n2); index.push_back(n3);

    Topic t1("1",10), t2("2",10), t3("3", 10);
    t1.addConsumer(n1); t1.addConsumer(n2);
    t2.addConsumer(n2); t2.addConsumer(n3);
    t3.addConsumer(n1); t3.addConsumer(n3);

    Message m1(1,t1,"h1"), m2(2,t2,"h2"), m3(3,t3,"h3");
    list<Message> lis;
    lis.push_back(m1); lis.push_back(m2); lis.push_back(m3);

    Filter filter(index);
    map<IpNode, list<Message> > ans = filter.filter(lis);
    for(auto it : ans)
    {
        for(auto that : it.second)
        {
            cout<<"Message : "<<that.getMessage()<<endl;
        }
        cout<<endl;
    }


    return 0;
}*/
