//
// Created by ironzhou on 2020/7/24.
//
//#include "../../include/Broker/MyQueue.h"
#include "../../include/ALL.h"

/*
template<class Archive>
void MyQueue::serialize(Archive & ar, const unsigned int version)
{
    serializationQueue.clear();

    queueMutex.mutex->lock();
    //queueMutex.lock();

    while(!queue.empty()) // 为了序列化先全部排出，并发问题怎么办？加锁
    {
        Message tm;
        queue.try_pop(tm); // try_pop会不会出问题？加锁了应该不会
        serializationQueue.push_back(tm);
    }
    for(auto it : serializationQueue) // 上面排出的添加回去
    {
        queue.push(it);
    }

    queueMutex.mutex->unlock();
    //queueMutex.unlock();

    ar & serializationQueue;
}
*/

int MyQueue::getSize()
{
    return queue.size();
}

void MyQueue::push(Message message)
{
    queue.push(message);
}

Message MyQueue::get()
{
    Message m(-1,-1);

    queueMutex.mutex->lock();

    queue.try_pop(m);
    if(m.getNum() != -1 && m.getType() != -1)
    {
        queue.push(m);
    }

    queueMutex.mutex->unlock();

    return m;
}

Message MyQueue::getAndRemove()
{
    Message m(-1,-1);

    queueMutex.mutex->lock(); // 这个锁不加 序列化可能出问题 丢失消息

    queue.try_pop(m);

    queueMutex.mutex->unlock();

    return m;
}

void MyQueue::getAll()
{
    // 构造时线程不安全,可能出错
    queueMutex.mutex->lock();

    tbb::concurrent_priority_queue<Message> temp = queue;

    queueMutex.mutex->unlock();

    while(!temp.empty())
    {
        Message m;
        temp.try_pop(m);
        cout<<"Message "<<m.getNum()<<", "<<m.getMessage()<<endl;
    }
    cout<<endl;
}

concurrent_priority_queue<Message> & MyQueue::getQueue()
{
    return queue;
}

vector<Message> MyQueue::getSerializationQueue()
{
    return serializationQueue;
}