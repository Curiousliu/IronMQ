//
// Created by ironzhou on 2020/8/4.
//


//#include "../../include/Utils/ResponseProcessor.h"
#include "../../include/ALL.h"

ResponseProcessor::ResponseProcessor() {}

ResponseProcessor::ResponseProcessor(int type) :type(type){}

int ResponseProcessor::getType() {return type;}

void ResponseProcessor::setType(int type) { this->type = type;}

void ResponseProcessor::processResponse(int epfd, struct epoll_event ev)
{
    struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
    string str = data->data;
    cout<<"服务器收到 : "<< str<<endl;
    string reply = "Ack " + str;
    Write(data->fd, reply);

    ChangeStateAndRelease(epfd, ev);
    
    return;
}

void ResponseProcessor::processResponse(int epfd, struct epoll_event ev, Broker & broker)
{
    struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
    string str = data->data;
    Message *msg = SerializeUtil::anti_serialize_to_Message(str);
    int fd = data->fd;

    if(msg->getType() == MessageType::REQUEST_QUEUE)
    {
        //cout<<"msg->getType() == MessageType::REQUEST_QUEUE "<<endl;
        broker.syncCreateQueue(msg->getTopic().getQueueNum(), msg->getTopic());
        broker.addTopicConsumer(msg->getTopic());

        string reply = "Ack " +to_string(msg->getNum()+1);
        reply += " , Get Queue :";

        map<Topic, set<string> > tempMap = broker.getTopicQueueMap();
        Topic tempTopic = msg->getTopic();
        set<string> tempSet = tempMap[tempTopic];
        for(string it : tempSet)
        //for(auto it : (broker.getTopicQueueMap())[msg->getTopic()])
        {
            //cout<<"string值："<<it<<endl;
            reply += " "+it;
        }

        reply += ", Assigned Consumer :";
        for(auto it : (broker.getTopicConsumerMap())[msg->getTopic()])
        {
            reply += " "+it.getIp()+":"+to_string(it.getPort());
            cout<<"Assigned Consumer : "<<it.getIp()<<":"<<to_string(it.getPort())<<" "<<endl;
        }
        Write(fd, reply);

    }
    else if(msg->getType() == MessageType::REPLY_EXPECTED)
    {
        //cout<<"Receive Msg Reply_expected"<<endl;
        addToBroker(*msg, broker);
        string reply = "Ack " +to_string(msg->getNum()+1);
        Write(fd, reply);
    }
    else if(msg->getType() == MessageType::ONE_WAY)
    {
        addToBroker(*msg, broker);
    }
    else if(msg->getType() == MessageType::REGISTER)
    {
        struct sockaddr consumerAddr;
        socklen_t len = sizeof(consumerAddr);
        if(getpeername(fd, &consumerAddr, &len) == 0)
        {
            struct sockaddr_in *sin = (struct sockaddr_in*)&consumerAddr;
            string ip = to_string(inet_ntoa(sin->sin_addr));
            int port = ntohs(sin->sin_port);
            cout<<"ip : "<<ip<<"   port : "<<port<<endl;
            IpNode ipNode(ip,port);
            broker.addConsumer(ipNode, fd);
        }
        string reply = "Ack " + msg->getMessage();
        Write(fd, reply);

        // Consumer 注册完从epoll监听中删除
        if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) != 0){
            cout<<"msg->getType() == MessageType::REGISTER. Epoll CTL Delete Error:"<<strerror(errno)<<endl;
            exit(-1);
        }

    }
    else if(msg->getType() == MessageType::PULL)
    {
        string reply = "Ack " + msg->getMessage();
        Write(fd, reply);
    }
    else if(msg->getType() == MessageType::ADD_TOPIC_CONSUMER)
    {
        broker.addTopicConsumer(msg->getTopic());
        string reply = "Ack ";
    }
    else if(msg->getType() == MessageType::DELETE_TOPIC_CONSUMER)
    {
        broker.deleteTopicConsumer(msg->getTopic());
    }
    else
    {
        string reply = "Ack MessageType Not Match";
        Write(fd, reply);
    }

    if(msg->getType() != MessageType::REGISTER)
        ChangeStateAndRelease(epfd, ev);

    return;
}


void ResponseProcessor::processResponse(int epfd, struct epoll_event ev, Slave & slave)
{
    struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
    string str = data->data;
    *(slave.broker) = SerializeUtil::anti_serialize_to_Broker(str);
    string reply = "Ack : receive Broker";
    Write(data->fd, reply);

    ChangeStateAndRelease(epfd, ev);
}

void ResponseProcessor::processResponse(int epfd, struct epoll_event ev, NameServer & nameServer)
{
    struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
    string str = data->data;
    Message *msg = SerializeUtil::anti_serialize_to_Message(str);
    int fd = data->fd;

    if(msg->getType() == MessageType::BROKER_HEART_BEAT)
    {
        // 接收心跳包 更新信息
        struct sockaddr brokerAddr;
        socklen_t len = sizeof(brokerAddr);
        if(getpeername(fd, &brokerAddr, &len) == 0)
        {
            struct sockaddr_in *sin = (struct sockaddr_in*)&brokerAddr;
            string ip = to_string(inet_ntoa(sin->sin_addr));
            int port = ntohs(sin->sin_port);
            cout<<"Broker ip : "<<ip<<"   port : "<<port<<endl;
            IpNode ipNode(ip,port);

            nameServer.Brokers[ipNode] = make_pair(time(0), true);
            nameServer.BrokerClients[ipNode] = fd;
            // 心跳包中的msg信息是序列化后的Slave IpNode
            nameServer.Broker_Slave[ipNode] = SerializeUtil::anti_serialize_to_IpNode(msg->getMessage());
        }
    }
    else if(msg->getType() == MessageType::REPLY_EXPECTED)
    {
        bool flag = false;
        // 轮询并更新broker的stat，然后转发给其中一个broker
        for(auto it : nameServer.Brokers)
        {
            time_t now = time(0);
            if(difftime(now, it.second.first) > DownTime)
            {
                it.second.second = false;
                nameServer.BrokerClients.erase(it.first);
            }
            if(it.second.second == true)
            {
                int BrokerFd = nameServer.BrokerClients[it.first];
                Write(BrokerFd, str);
                // 改变状态 set to read
                struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
                ev.data.fd = BrokerFd;
                ev.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, BrokerFd, &ev) != 0) {
                    cout<<"Epoll Write-Read state Changing Error:"<<strerror(errno)<<endl;
                    exit(-1);
                }
                free(data->data);
                free(data);
                flag = true;
                break;
            }
        }
        if(!flag) // 所有broker都宕机，切热备
        {
            int i = 0;
            for(auto it : nameServer.Broker_Slave)
            {
                Client client(it.second.getIp(), it.second.getPort());
                if(client.getClientFd() < 0)
                {
                    cout<<"连接失败"<<endl;
                }
                string ack = client.SyncSend(*msg);
                if(ack.find("Ack") != string::npos)
                {
                    flag = true;
                    cout<<"热备发送成功 : "<<ack<<endl;
                    nameServer.BrokerClients[it.second] = client.getClientFd();
                    break;
                }
                else
                {
                    cout<<"热备"<<i<<"发送失败, ret : "<<ack<<endl;
                }
            }
        }

        string reply;
        if(flag)
        {
            reply = "Ack " +to_string(msg->getNum()+1);
        }
        else
        {
            reply = "Failed " +to_string(msg->getNum()+1);
        }
        Write(fd, reply);
    }
    /*else if(msg->getType() == MessageType::REGISTER || msg->getType() == MessageType::REQUEST_QUEUE ||
        msg->getType() == MessageType::ADD_TOPIC_CONSUMER || msg->getType() == MessageType::DELETE_TOPIC_CONSUMER)
    {   // consumer注册消息以及Topic_request_queue注册消息
        // 转发给每个Broker
        for(auto it : nameServer.Brokers)
        {
            time_t now = time(0);
            if(difftime(now, it.second.first) > DownTime)
            {
                it.second.second = false;
                nameServer.BrokerClients.erase(it.first);
            }
            if(it.second.second == true)
            {
                int BrokerFd = nameServer.BrokerClients[it.first];
                Write(BrokerFd, str);
                // 改变状态 set to read
                struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
                ev.data.fd = BrokerFd;
                ev.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, BrokerFd, &ev) != 0) {
                    cout<<"Epoll Write-Read state Changing Error:"<<strerror(errno)<<endl;
                    exit(-1);
                }
                free(data->data);
                free(data);
                break;
            }
        }
    }*/
    else
    {
        string reply = "Ack. But MessageType Not Match";
        Write(fd, reply);
    }

    ChangeStateAndRelease(epfd, ev);
}

void ResponseProcessor::addToBroker(Message msg, Broker & broker)
{
    set<string> s = (broker.getTopicQueueMap())[msg.getTopic()];
    int queueNum = LoadBalancer::balancer(msg.getTopic(), broker.getTopicQueueMap(), broker.getQueueList());
    broker.syncAdd(queueNum, msg);
    /*cout<<"ResponseProcessor::addToBroker GetAll Start"<<endl;
    broker.getAll();
    cout<<"ResponseProcessor::addToBroker GetAll Exit"<<endl;*/
}

void ResponseProcessor::Write(int fd, string reply)
{
    int ret = 0;
    int send_pos = 0;
    char* send_buf = (char*)malloc(BufferSize);
    //char* send_buf = new char(reply.size());
    memcpy(send_buf, reply.c_str(), reply.size());
    const int total = strlen(send_buf);
    while(true)
    {
        ret = write(fd, (send_buf + send_pos), total - send_pos);
        if (ret < 0) {
            if (errno == EAGAIN) {
                sched_yield();
                continue;
            }
            cout<<"Write Error:"<<strerror(errno)<<endl;
            exit(-1);
        }
        send_pos += ret;
        if (total == send_pos) {
            break;
        }
    }
}

void ResponseProcessor::ChangeStateAndRelease(int epfd, struct epoll_event ev)
{
    struct Epoll_Data* data = (struct Epoll_Data*)ev.data.ptr;
    //set to read
    ev.data.fd = data->fd;
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, data->fd, &ev) != 0) {
        cout<<"Epoll Write-Read state Changing Error:"<<strerror(errno)<<endl;
        exit(-1);
    }

    free(data->data);
    free(data);
}