//
// Created by ironzhou on 2020/7/27.
//
//#include "../../include/Broker/LoadBalancer.h"
#include "../../include/ALL.h"

struct cmp
{
    bool operator()(string_size_map a, string_size_map b)
    {
        return a.Size > b.Size;
    }
};

int LoadBalancer::balancer(Topic topic, map<Topic, set<string> > topicQueueMap, concurrent_hash_map<string, MyQueue> queueList)
{
    int queueNum = 0;
    if(topicQueueMap[topic].empty())
    {
        cout<<"Topic : "<<topic.getTopicName()<<" has no assigned Myqueue, default assigned to Myqueue 0"<<endl;
        return queueNum;
    }
    priority_queue<string_size_map, vector<string_size_map>, cmp> priorityQueue;
    for(auto it : topicQueueMap[topic])
    {
        concurrent_hash_map<string, MyQueue>::accessor a;
        queueList.insert(a,it);
        priorityQueue.push(string_size_map(it, a->second.getSize()));
        a.release();
    }
    queueNum = stoi(priorityQueue.top().Num);
    return queueNum;
}

