//
// Created by ironzhou on 2020/7/24.
//
/*
#include "../../include/Broker/Broker.h"
#include "../../include/Utils/Server.h"
*/
#include "../../include/ALL.h"

void Broker::init(int port)
{
    cout << "Broker已启动" << endl;

    // 监听生产者
    RequestProcessor RequestProcessor;
    ResponseProcessor brokerResponseProcessor(0);
    Slave slave1;

    auto _broker = [&](){
        new Server(port, RequestProcessor, brokerResponseProcessor, *this, slave1);
    };

    // slave同步
    auto _slave = [&](){
        map<IpNode, int> slaves;
        while(true)
        {
            if(hasSlave)
            {
                try{
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(sync_Time));
                }
                catch (std::exception e) {
                    cerr<<e.what();
                }

                try{
                    string s = SerializeUtil::serialize_Broker(*this);
                    cout<<"序列化Broker:"<<s<<" 大小："<<s.size()<<endl;
                    for(auto ip = slave.begin() ; ip != slave.end();)
                    {
                        Client *client;
                        if(slaves.count(*ip))
                        {
                            client = new Client(slaves[*ip]);
                        }
                        else
                        {
                            IpNode tempIp = *ip;
                            client = new Client(tempIp.getIp(), tempIp.getPort());
                            if(client->getConnect() == false)
                            {
                                cout<<"slave 未上线, Ip = "<<tempIp.getIp()<<":" <<tempIp.getPort()<<endl;
                                slave.erase(ip++);
                                continue;
                            }
                            slaves[tempIp] = client->getClientFd();
                        }

                        string ack = "";
                        for(int i = 0;i<16;i++)
                        {

                            try{
                                boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
                            }
                            catch (std::exception e) {
                                cerr<<e.what();
                            }

                            if(client != NULL)
                                ack = client->SyncSend(s);
                            if(ack.find("Ack") != string::npos)
                            {
                                break;
                            }
                        }

                        if(ack.find("Ack") == string::npos) // 认为该slave断开连接 删除
                        {
                            slaves.erase(*ip);
                        }
                        else
                            cout<<ack<<endl; // test slave

                        ip++;
                    }
                }
                catch (std::exception e) {
                    cout<<"Slave didn't work"<<endl;
                }
            }
        }
    };


    // 持久化
    auto _persistence = [&](){
        while(true)
        {
            if(startPersistence)
            {
                try{
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(store_Time));
                }
                catch (std::exception e) {
                    cerr << e.what();
                }
            }
            try{
                string path = "../File/";
                PersistenceUtil::BrokerPersistence(*this, path);
            }
            catch (std::exception e) {
                cerr << e.what();
            }
        }
    };

    auto _push = [&](){
        deadQueue.clear();
        while(true)
        {
            if(pushMode)
            {
                try {
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(push_Time));
                }
                catch (std::exception e) {
                    cerr << e.what();
                }
                push();
            }
        }
    };

    thread thrd(_broker);
    thread thrd2(_slave);
    thread thrd3(_persistence);
    thread thrd4(_push);

    auto f = [&]()
    {

    };



    /*auto test = [&](){
        //test
        while(true)
        {
            boost::this_thread::sleep_for(boost::chrono::milliseconds(10000));
            cout<<"Broker::init(int port) GetAll Start"<<endl;
            (*this).getAll();
            cout<<"Broker::init(int port) GetAll Exit"<<endl;
        }
    };
    thread thrdTest(test);*/
}

void Broker::syncCreateQueue(int queueNum, Topic topic)
{
    for(int i = 0; i < queueNum ; i++)
    {
        concurrent_hash_map<string, MyQueue>::accessor a;
        int temp_count = count;
        count += 1;
        MyQueue queue;
        queueList.insert(a,make_pair(to_string(temp_count), queue));
        a.release();
        // 添加映射关系
        string temp_str = to_string(temp_count);
        topicQueueMap[topic].insert(temp_str);
    }
    /*cout<<"Broker::syncCreateQueue(int queueNum, Topic topic) start"<<endl;
    for(auto it : topicQueueMap)
    {
        Topic tt = it.first;
        cout<<tt.getTopicName()<<endl;
        for(auto that : it.second)
        {
            cout<<that<<" ";
        }
    }
    cout<<endl<<"Broker::syncCreateQueue(int queueNum, Topic topic) exit"<<endl;*/
}

list<Message> Broker::syncPoll(int num/*设置拉取轮数*/)
{
    list<Message> l;

    for(int i = 0 ; i < num ; i++)
    {
        for(auto it : queueList)
        {
            concurrent_hash_map<string, MyQueue>::accessor a;
            queueList.insert(a, it.first);
            if(a->second.getSize() <= 0 )
                continue;
            Message message = a->second.getAndRemove();
            a.release();
            if(message.getType() != -1 && message.getNum() != -1)
            {
                l.push_back(message);
            }
        }
    }

    return l;
}

map<IpNode, set<Message> > Broker::create_filter(list<Message> list)
{
    /*if(!list.empty())
        cout<<"Broker::create_filter : list<Message>"<<endl;
    for(auto it : list)
    {
        cout<<it.getNum()<<" "<<it.getMessage()<<" "<<it.getTopic().getTopicName()<<endl;
    }*/
    map<IpNode, set<Message> > temp = filter.classification(clients, list);
    /*if(!temp.empty())
        cout<<"Broker::create_filter : map<IpNode, set<Message> >"<<endl;
    for(auto it : temp)
    {
        IpNode tempIp = it.first;
        cout<<tempIp.getIp()<<":"<<tempIp.getPort()<<endl;
        for(auto that : it.second)
        {
            cout<<that.getNum()<<" "<<that.getMessage()<<" "<<that.getTopic().getTopicName()<<endl;
        }
    }*/
    return temp;
}

Broker::Broker() {}

Broker::Broker(int port)
{
    init(port);
}

Broker::Broker(int port, set<IpNode> slave)
{
    this->slave = slave;
    hasSlave = true;
    init(port);
}

bool Broker::syncAdd(int queueNum, Message message)
{
    concurrent_hash_map<string, MyQueue>::accessor a;
    queueList.insert(a, to_string(queueNum));
    a->second.push(message);
    a.release();
    return true;
}

void Broker::setStartPersistence(bool startPersistence)
{
    // 是否开启持久化
    this->startPersistence = startPersistence;
}

void Broker::setQueueList(concurrent_hash_map<string, MyQueue> queueList)
{
    // 设置队列内容
    this->queueList = queueList;
}

void Broker::setStoreTime(int store_Time)
{
    // 设置刷盘时间
    this->store_Time = store_Time;
}

void Broker::setSyncTime(int sync_Time)
{
    // 设置同步时间
    this->sync_Time = sync_Time;
}

void Broker::setPushTime(int time)
{
    // 设置push时间间隔
    this->push_Time = time;
}

void Broker::setRetryTime(int time)
{
    // 设置重试时间间隔
    this->reTry_Time = time;
}

void Broker::setPushMode(bool b)
{
    pushMode = b;
}

void Broker::getAll()
{
    for(auto it : queueList)
    {
        cout<<"Number of MyQueue"<<it.first<<endl;
        it.second.getAll();
    }
}

Broker Broker::recover()
{
    string path = "../File/";
    return PersistenceUtil::BrokerRecovery(path);
}

void Broker::addConsumer(IpNode ipNode,int fd)
{
    index.insert(ipNode);
    clients[ipNode] = fd;
    //cout<<"Add Consumer Succeed"<<endl;
}

void Broker::deleteConsumer(IpNode ipNode)
{
    auto it = find(index.begin(), index.end(), ipNode);
    if(it != index.end())
    {
        index.erase(it);
    }
    if(clients.count(ipNode))
    {
        close(clients[ipNode]);
        clients.erase(ipNode);
    }
}

void Broker::addTopicConsumer(Topic topic)
{
    //cout<<"Broker::addTopicConsumer(Topic topic)"<<endl;
    for(auto it : topic.getConsumers())
    {
        if(find(filter.topicConsumerMap[topic].begin(), filter.topicConsumerMap[topic].end(), it) != filter.topicConsumerMap[topic].end()) // 映射关系已存在
        {
            cout<<"Add Topic Consumer Failed: 映射关系已存在, Consumer IpNode = "<<it.getIp()<<":"<<it.getPort()<<endl;
            continue;
        }
        if(index.count(it) == 0) // 消费者未注册
        {
            cout<<"Add Topic Consumer Failed: 消费者未注册, Consumer IpNode = "<<it.getIp()<<":"<<it.getPort()<<endl;
            continue;
        }
        //IpNode tt = it;
        //cout<<tt.getIp()<<":"<<tt.getPort()<<endl;
        filter.topicConsumerMap[topic].insert(it);
    }
    /* test
    for(auto it : topicQueueMap)
    {
        Topic topic1 = it.first;
        cout<<"Topic "<< topic1.getTopicName()<<endl;
        for(auto that : it.second)
        {
            cout<<that<<" ";
        }
    }*/
}

void Broker::deleteTopicConsumer(Topic topic)
{
    for(auto it : topic.getConsumers())
    {
        if(find(filter.topicConsumerMap[topic].begin(), filter.topicConsumerMap[topic].end(), it) == filter.topicConsumerMap[topic].end()) // 映射关系未存在
        {
            cout<<"Delete Topic Consumer Failed: 映射关系不存在, Consumer IpNode = "<<it.getIp()<<":"<<it.getPort()<<endl;
            continue;
        }
        filter.topicConsumerMap[topic].erase(it);
    }
}

void Broker::pushMessage()
{
    // 为消费者推送消息
    map<IpNode, set<Message> > t_map = create_filter(syncPoll(1));
    for(auto it : t_map)
    {
        if(clients.count(it.first))
        {
            Client client(clients[it.first]);

            for(Message that : it.second)
            //for(auto that = it.second.begin(); that != it.second.end() ;that++)
            {
                //Message tempMsg = that;
                //cout<<tempMsg.getNum()<<" "<<tempMsg.getMessage()<<endl;
                if(deadQueue.count(that.getNum() == 0))
                    deadQueue.insert(make_pair(that.getNum(), that));
                string ack = client.SyncSend(that);
                if(ack.find("Ack") != string::npos)
                {
                    string temp = ack.substr(ack.find(" "));
                    int num = stoi(temp) - 1;
                    if(deadQueue.count(num))
                    {
                        deadQueue.erase(num);
                    }
                }
            }

            int i = 0;
            // 重发
            while (i++ < reTry_Time && !deadQueue.empty())
            {
                for(map<int, Message>::iterator k = deadQueue.begin() ; k != deadQueue.end();)
                {
                    string ack = client.SyncSend(k->second);
                    if(ack.find("Ack") != string::npos)
                    {
                        string temp = ack.substr(ack.find(" "));
                        int num = stoi(temp) - 1;
                        auto t = deadQueue.find(num);
                        if(t != deadQueue.end())
                        {
                            if(k == t)
                            {
                                deadQueue.erase(k++);
                            }
                            else
                            {
                                deadQueue.erase(t);
                            }
                        }
                        else
                        {
                            k++;
                        }
                    }
                }
            }
            if(i >= reTry_Time)
            {
                //todo 进入死信队列
                cout<<"死信队列大小:"<<deadQueue.size()<<endl;
            }
        }
        else
        {
            IpNode temp = it.first;
            cout<<"Push Message Error : Consumer Connection is not existed, IpNode = "<<temp.getIp()<<":"<<temp.getPort()<<endl;
        }
    }
}

void Broker::push()
{
    //cout<<"push Message"<<endl;
    pushMessage();
}

void Broker::pullMessage(IpNode ipNode)
{
    // pull模式
    list<Message> l;
    // 查找队列最外层消息，找到对应ipNode的消息
    for(auto queue : queueList)
    {
        // 消息出队
        Message m = queue.second.getAndRemove();
        if(m.getNum() != -1 && m.getType() != -1)
        {
            set<IpNode> lp = m.getTopic().getConsumers();
            auto that = find(lp.begin(), lp.end(), ipNode);

            if(that != lp.end() && lp.size() == 1)
            {
                // 只有一个消费者
                l.push_back(m);
            }
            else if(that != lp.end() && lp.size() > 1)
            {
                // 多个消费者，则删除这个消费者，并将该消息推送给它，再入队给其他消费者使用
                m.getTopic().deleteConsumer(ipNode);
                l.push_back(m);
                queue.second.push(m);
            }
        }
    }

    for(auto m : l)
    {
        try{
            Client *client = new Client(ipNode.getIp(), ipNode.getPort());
            if(client != NULL)
            {
                int i = 0;
                for(; i < reTry_Time ; i++)
                {
                    string ack = "";
                    try{
                        ack = client->SyncSend(m);
                    }
                    catch (std::exception e) {
                        cout << "Push Message Failed : Retrying the " << i << "th time" << endl;
                    }
                    if(ack != "" && ack != "Failed")
                    {
                        break;
                    }
                }
            }
        }
        catch (std::exception e) {
            cout << "Consumer is not existed" << endl;
        }
    }
}

map<Topic, set<string> > Broker::getTopicQueueMap()
{
    return topicQueueMap;
}

concurrent_hash_map<string, MyQueue> & Broker::getQueueList()
{
    return queueList;
}

map<Topic, set<IpNode> > Broker::getTopicConsumerMap()
{
    return filter.getTopicConsumerMap();
}

void Broker::setSlave(set<IpNode> slaves)
{
    slave = slaves;
    if(!slave.empty())
        hasSlave = true;
}
