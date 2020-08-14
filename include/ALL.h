//
// Created by ironzhou on 2020/8/5.
//

#ifndef MYMQ_ALL_H
#define MYMQ_ALL_H
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/synchronized_value.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/bind.hpp>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_priority_queue.h>
#include "json/json.h"
/*#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"*/
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <ctime>
#include <exception>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#define BufferSize 3500
#define SlaveBufSize 4096
#define ServerIP "0.0.0.0"
#define serverPort 23333
#define slavePort  23339
#define ListenBacklog SOMAXCONN
#define EpollSize 60000
#define consumerPort1 36936
#define consumerPort2 36939
#define consumerPort3 36963
#define consumerPort4 36969
#define DownTime 60

#define TOTAL_MESSAGE_NUM 100000
// 消费2万条消息      4s
// 消费20万条消息     44s
// 消费200万条消息    450s

using namespace std;
using namespace boost;
using namespace tbb;

class Broker;
class PersistenceUtil;
class SerializeUtil;
class ResponseProcessor;

enum MessageType{
    ONE_WAY        = 0,
    REPLY_EXPECTED = 1,
    REQUEST_QUEUE  = 2,
    REGISTER       = 3,
    PULL           = 4,
    ADD_TOPIC_CONSUMER      = 5,
    DELETE_TOPIC_CONSUMER   = 6,
    BROKER_HEART_BEAT       = 7
};

void split(const std::string& s, std::vector<std::string>& sv, const char* delim = " ");

class IpNode{
private:
    int port = 0;
    string ip = "127.0.0.1";

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & port & ip;
    }

public:
    IpNode();
    IpNode(string i, int p) ;
    string getIp();
    void setIp(string s);
    int getPort();
    void setPort(int p);
    bool operator==(const IpNode& compareTo) const;
    bool operator<(const IpNode& compareTo) const;
};

class Topic{
private:
    //存储结构都为Set，为了方便去重和快速查找
    int queueNum = 0;                   // 请求队列数
    string topicName ;                  // 主题名称
    set<IpNode> consumer_address;       // 该Topic对应consumer

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & queueNum & topicName & consumer_address;
    }

public:
    Topic();
    Topic(string s);
    Topic(string s, int queueNum);
    Topic(string s, int queueNum, set<IpNode> consumer_addr);
    //Topic( const Topic &topic);

    //set转list
    string getTopicName();
    //list<int> getQueue();
    set<IpNode> getConsumers();
    void addConsumer(IpNode i);
    void deleteConsumer(IpNode i);
    //void addQueueId(int i);
    int getQueueNum();

    bool operator<(const Topic & t)const;
};

class Message{
private:
    int num = 0;                // 消息序号
    int type = 0;               // 消息类型
    string message = "";        // 消息
    int priority = 0;           // 优先级
    Topic topic;                // 消息主题

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & num & type & message & priority & topic;
    }

public:
    Message();
    Message(int num, int type);
    Message(int num, Topic topic1, string s);
    Message(int num, int type, string s);
    Message(int num, int type, Topic topic1, string s);
    //Message(const Message & msg);

    string getMessage();
    int getType();
    void setType(int type);
    Topic getTopic();
    void setTopic(Topic top);
    int getNum();
    void setNum(int num);
    void setPriority(int num);
    int getPriority();
    bool operator<(const Message & m)const;

};

class MyMutex{
public:
    boost::mutex *mutex;
    MyMutex();
    ~MyMutex();

    MyMutex(MyMutex const & mx);
    MyMutex& operator=(MyMutex const & mx);
};

class MyQueue{
private:
    //volatile static int count = 1;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
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

    MyMutex queueMutex;
    //boost::mutex queueMutex;

    concurrent_priority_queue<Message> queue;       // 都加锁了要不用priority_queue?
    vector<Message> serializationQueue;             // 用来（反）序列化存储queue


public:
    MyQueue(){}
    ~MyQueue(){}
    int getSize();
    void push(Message message);
    Message get();
    Message getAndRemove();
    void getAll();

    concurrent_priority_queue<Message> & getQueue();
    vector<Message> getSerializationQueue();

};

class Filter{
private:
    map<Topic, set<IpNode> > topicConsumerMap;          // Topic和消费者映射
    friend class Broker;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & topicConsumerMap;
    }

public:
    Filter();
    Filter(map<Topic, set<IpNode> > Map);
    map<Topic, set<IpNode> > getTopicConsumerMap();
    map<IpNode, set<Message> > classification(map<IpNode, int> clients, list<Message> l);
};

struct string_size_map{
public:
    string Num;
    int Size;
    string_size_map(string num, int size):Num(num), Size(size){}
};

class LoadBalancer{
public:
    // 找到该Topic对应队列中 最小负载的队列
    static int balancer(Topic topic, map<Topic, set<string> > topicQueueMap, concurrent_hash_map<string, MyQueue> queueList);
};

class Client{
private:
    struct sockaddr_in serverAddr, clientAddr;
    volatile static int count;
    string ip;
    int port;
    char buf[BufferSize];
    int clientFd;
    bool Connect = false;

    bool init(string ip, int port);
    bool init(string ip, int port, int clientPort);

public:
    Client();
    Client(int fd);
    Client(string ip, int port);
    Client(string ip, int port, int clientPort);
    void Send(string str);
    string SyncSend(string str);
    string SyncSend(Message msg);
    string receive();
    int getClientFd();
    void setIP(string s);
    void setPort(int p);
    bool getConnect();
};

/*class Synchronizer{
private:
    concurrent_hash_map<string, MyQueue> queuelist;
    map<string, MyQueue> serializationMap;         // （反）序列化时存储queuelist
    set<IpNode> index;                             // 消费者地址

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        serializationMap.clear();
        for(auto it : queuelist)
        {
            serializationMap.insert(make_pair(it.first,it.second));
        }
        ar & index & serializationMap;
    }

public:
    Synchronizer();
    Synchronizer(concurrent_hash_map<string, MyQueue> qlist, set<IpNode> index);
    concurrent_hash_map<string, MyQueue> getQueueList();
    set<IpNode> getIndex();
    map<string, MyQueue> getSerializationMap();
    void addToConcurrentHashMap(string str, MyQueue myQueue);
    void anti_serialization();
};*/

class SerializeUtil{
public:
    static string serialize_Message(Message msg);
    //static string serialize_Synchronizer(Synchronizer obj);
    static Message* anti_serialize_to_Message(string str);
    //static Synchronizer anti_serialize_to_Synchronizer(string str);

    static string serialize_IpNode(IpNode ipNode);
    static IpNode anti_serialize_to_IpNode(string str);

    static string serialize_Topic(Topic topic);
    static Topic anti_serialize_to_Topic(string str);

    static string serialize_Filter(Filter filter);
    static Filter anti_serialize_to_Filter(string str);

    static string serialize_MyQueue(MyQueue myQueue);
    static MyQueue anti_serialize_to_MyQueue(string str);

    static string serialize_Broker(Broker broker);
    static Broker anti_serialize_to_Broker(string str);
};

/*class JsonFormatUtil{
private:
    static string SPACE; // 单位缩进符

public:
    string formatJson(string json);

};*/

class PersistenceUtil{
private:

public:
    static void BrokerPersistence(Broker broker, string path);

    static Broker BrokerRecovery(string path);

    /*static void ConsumerPersistence(set<IpNode> consumerAddress, string path);
    static void ConfigurePersistence(int count, int push_Time, bool hasSlave,
                                     bool startPersistence, bool pushMode,  int sync_Time,
                                     int reTry_Time, int store_Time, string path);
    static void FilterPersistence(Filter filter, string path);



    static set<IpNode> ConsumerRecovery(string path);
    static map<string, int> ConfigureRecovery(string path);
    static Filter FilterRecovery(string path);*/
    //static rapidjson::Document persistentConsumer(set<IpNode> consumerAddress, string path);
    //static bool Export(rapidjson::Document document, string path);
};

class Broker{
private:
    int temp_count = 0;             // 用于序列化时存储count
    volatile int count = 0;         // 记录队列编号
    int push_Time = 1000;           // push时间默认一秒一次
    bool hasSlave = false;          // 是否有备份节点
    bool startPersistence = true;   // 是否开启持久化
    bool pushMode = true;           // 是否push模式
    int sync_Time = 1000;           // sync时间默认一秒一次
    int reTry_Time = 16;            // 发送失败重试次数
    int store_Time = 1000;          // 刷盘时间间隔

    concurrent_hash_map<string, MyQueue> queueList;     // 队列号和实际队列映射
    map<string, MyQueue> serializationQueueList;        // 用于序列化存储queueList
    map<Topic, set<string> > topicQueueMap;             // 记录Topic和队列号映射
    //map<Topic, set<IpNode> > topicConsumerMap;        // Topic和消费者映射
    Filter filter;                  // 过滤器
    set<IpNode> index;              // 消费者地址
    map<IpNode, int> clients;       // 地址和文件描述符映射
    set<IpNode> slave;
    map<int, Message> deadQueue;    // 死信队列

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        temp_count = count;
        ar & temp_count & push_Time & hasSlave & startPersistence & pushMode & sync_Time & reTry_Time & store_Time;
        serializationQueueList.clear();
        for(auto it : queueList) // boost::serialization不支tbb::concurrent_hash_map , 用map保存并序列化
        {
            concurrent_hash_map<string, MyQueue>::accessor a;
            queueList.insert(a, it.first);
            serializationQueueList.insert(make_pair(a->first, a->second));
            a.release();
        }
        ar & serializationQueueList;
        ar & topicQueueMap & filter & index & clients & slave & deadQueue;
    }

    friend class PersistenceUtil;
    friend class SerializeUtil;
    friend class ResponseProcessor;

    void init(int port);

    // 队列出队，所有队列均出队一个元素
    list<Message> syncPoll(int num/*设置拉取轮数*/);
    //过滤
    map<IpNode, set<Message> > create_filter(list<Message> list);

public:
    Broker();
    Broker(int port);
    Broker(int port, set<IpNode> slave);
    //Broker(const Broker & broker);

    map<Topic, set<string> > getTopicQueueMap();
    concurrent_hash_map<string, MyQueue> & getQueueList();
    map<Topic, set<IpNode> > getTopicConsumerMap();


    //创建队列
    void syncCreateQueue(int queueNum, Topic topic);
    // 是否开启持久化
    void setStartPersistence(bool startPersistence);
    // 设置队列内容
    void setQueueList(concurrent_hash_map<string, MyQueue> queueList);
    // 设置刷盘时间
    void setStoreTime(int store_Time);
    // 设置同步时间
    void setSyncTime(int sync_Time);
    // 设置Push间隔
    void setPushTime(int time);
    // 设置重试时间间隔
    void setRetryTime(int time);
    // 是否push
    void setPushMode(bool b);
    void getAll();
    // 恢复Broker
    Broker recover();
    // 添加消费者
    void addConsumer(IpNode ipNode, int fd);
    // 删除消费者
    void deleteConsumer(IpNode ipNode);
    // 添加Topic的消费者
    void addTopicConsumer(Topic topic);
    // 删除Topic的消费者
    void deleteTopicConsumer(Topic topic);
    // 为消费者推送消息
    void pushMessage();
    // push模式
    void push();
    // pull模式
    void pullMessage(IpNode ipNode);
    // 将消息添加到某个队列中, 若队列号不存在返回false
    bool syncAdd(int queueNum, Message value);

    // setHasSlave 用于持久化恢复
    void setHasSlave(bool b);
    // setIndex 用于持久化恢复
    void setIndex(set<IpNode> ips);
    // setSlave 用于持久化
    void setSlave(set<IpNode> slaves);
    // setClients
    void setClients(map<IpNode, int> clts);
    // set topicQueueMap
    void setTopicQueueMap(map<Topic, set<string> > tqm);

};

class NameServer{
private:
    map<IpNode, pair<time_t, bool> > Brokers;   // Broker地址和状态关系(最后一次心跳包时间， 是否可用)
    map<IpNode, IpNode> Broker_Slave;           // 热备
    map<IpNode, int>  BrokerClients;            // IpNode - fd

    friend class ResponseProcessor;

public:
    NameServer(int port);


};


struct Epoll_Data {
    char* data;
    int fd;
};

class RequestProcessor{
private:
    //static boost::thread_group threadGroup;

public:
    //virtual void processRequest(Server server) = 0;
    RequestProcessor();
    void processRequest(int epfd, struct epoll_event ev);
    //void processRequest(int epfd, struct epoll_event ev, int type);

};

class Slave{
private:

public:
    Broker *broker;
    Slave();
    Slave(int port/*slave监听端口*/);
};

class ConsumerMessage{
private:
    int num;
    int priority;
    string msg;

public:
    ConsumerMessage();
    ConsumerMessage(Message msg);
    void setNum(int Num);
    int getNum();
    //string getTopic();
    int getPriority();
    string getMsg();
    bool operator<(const ConsumerMessage cmsg)const;
};

class Consumer{
private:
    int count;
    int fd = -1;
    int port = -1;
    char buf[BufferSize];
    Client *client;
    priority_queue<ConsumerMessage> priorityQueue;
    set<ConsumerMessage> checkMsg;

    time_t t_start, t_end;

public:
    Consumer(int num);
    void Register(IpNode ipNode/*目标地址*/);
    void Register(IpNode ipNode, int clientPort); // 指定clientPort，方便topicRegister注册
    void listening();
    //void EventHandle(int epfd, struct epoll_event ev);
    time_t getTend();
};

class ResponseProcessor{
private:
    int type;
    void Write(int fd, string reply);
    void ChangeStateAndRelease(int epfd, struct epoll_event ev);

public:
    ResponseProcessor();
    ResponseProcessor(int type);
    void setType(int type);
    int getType();
    void addToBroker(Message msg, Broker & broker);
    void processResponse(int epfd, struct epoll_event ev);
    void processResponse(int epfd, struct epoll_event ev, NameServer & nameServer);
    void processResponse(int epfd, struct epoll_event ev, Broker & broker);
    void processResponse(int epfd, struct epoll_event ev, Slave & slave);
};

class Server{
private:
    RequestProcessor requestProcessor;
    ResponseProcessor responseProcessor;
    Broker & broker;
    Slave & slave;
    int serverFd, clientFd, epollFd;
    struct sockaddr_in serverAddr, clientAddr;
    struct epoll_event epollEvents[EpollSize];
    NameServer* nameServer = NULL;

public:
    Server(int port,RequestProcessor requestProcessor,ResponseProcessor responseProcessor, Broker & broker, Slave & slave, NameServer* nameServer1);
    Server(int port,RequestProcessor requestProcessor,ResponseProcessor responseProcessor,Broker & broker, Slave & slave);
    //Server(int port,RequestProcessor requestProcessor,ResponseProcessor responseProcessor,Slave & slave);

    void init(int port);
    void addFd(int epollFd, int fd);
    void start(int port);
    void EventHandle(int epfd, struct epoll_event ev);
};

class SequenceUtil{
private:
    volatile int count = 0;
    boost::mutex sequenceMutex;

public:
    int getSequence();

};

class TopicRegister{
private:
    Topic topic;
    IpNode ipNode;
    void Register();


public:
    TopicRegister();
    TopicRegister(Topic topic, IpNode i);

};

class Producer{
private:
    static int reTry_Time;
    static Client client;

public:
    static void setRetryTime(int num);
    static void Send(Message msg, IpNode ipNode);
    static void Send(Message msg);
};

#endif //MYMQ_ALL_H
