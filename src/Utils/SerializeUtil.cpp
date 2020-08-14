//
// Created by ironzhou on 2020/8/3.
//

//#include "../../include/Utils/SerializeUtil.h"

#include "../../include/ALL.h"


string SerializeUtil::serialize_Message(Message msg)
{
    ostringstream s;
    archive::text_oarchive oa(s);
    oa << msg;
    return s.str();
}


Message* SerializeUtil::anti_serialize_to_Message(string str)
{
    Message *msg = new Message();
    istringstream is(str);
    archive::text_iarchive ia(is);
    ia >> *msg;
    return msg;
}

string SerializeUtil::serialize_IpNode(IpNode ipNode)
{
    ostringstream s;
    archive::text_oarchive oa(s);
    oa << ipNode;
    return s.str();
}

IpNode SerializeUtil::anti_serialize_to_IpNode(string str)
{
    IpNode ipNode;
    istringstream is(str);
    archive::text_iarchive ia(is);
    ia >> ipNode;
    return ipNode;
}

string SerializeUtil::serialize_Topic(Topic topic)
{
    ostringstream s;
    archive::text_oarchive oa(s);
    oa << topic;
    return s.str();
}

Topic SerializeUtil::anti_serialize_to_Topic(string str)
{
    Topic topic;
    istringstream is(str);
    archive::text_iarchive ia(is);
    ia >> topic;
    return topic;
}

string SerializeUtil::serialize_Filter(Filter filter)
{
    ostringstream s;
    archive::text_oarchive oa(s);
    oa << filter;
    return s.str();
}

Filter SerializeUtil::anti_serialize_to_Filter(string str)
{
    Filter filter;
    istringstream is(str);
    archive::text_iarchive ia(is);
    ia >> filter;
    return filter;
}

string SerializeUtil::serialize_MyQueue(MyQueue myQueue)
{
    ostringstream s;
    archive::text_oarchive oa(s);
    oa << myQueue;
    return s.str();
}

MyQueue SerializeUtil::anti_serialize_to_MyQueue(string str)
{
    MyQueue myQueue;
    istringstream is(str);
    archive::text_iarchive ia(is);
    ia >> myQueue;
    // 从SerializationQueue中反序列化 因为tbb不被支持
    concurrent_priority_queue<Message> & queue = myQueue.getQueue();
    for(auto it : myQueue.getSerializationQueue())
    {
        queue.push(it);
    }
    return myQueue;
}

string SerializeUtil::serialize_Broker(Broker broker)
{
    ostringstream s;
    archive::text_oarchive oa(s);
    oa << broker;
    return s.str();
}

Broker SerializeUtil::anti_serialize_to_Broker(string str)
{
    Broker broker;
    istringstream is(str);
    archive::text_iarchive ia(is);
    ia >> broker;
    broker.count = broker.temp_count;

    concurrent_hash_map<string, MyQueue> & queueList = broker.getQueueList();
    for(auto it : broker.serializationQueueList) // 还原queueList
    {
        // 还原MyQueue
        concurrent_priority_queue<Message> & myQueue = it.second.getQueue();
        for(auto that : it.second.getSerializationQueue())
        {
            myQueue.push(that);
        }
        queueList.insert(make_pair(it.first, it.second));
    }

    return broker;
}