//
// Created by ironzhou on 2020/8/6.
//
#include "../../include/ALL.h"


Consumer::Consumer(int num):count(num){}

void Consumer::Register(IpNode ipNode)
{
    auto f = [&](){
        cout<<"正在注册Consumer..."<<endl;
        try{
            Client client(ipNode.getIp(), ipNode.getPort());
            fd = client.getClientFd();
            IpNode localIP("",-1);
            if(fd < 0)
            {
                cout<<"连接失败"<<endl;
            }
            else
            {
                struct sockaddr local_addr;
                socklen_t len = sizeof(sockaddr);

                if(getsockname(fd, &local_addr, &len) == 0)
                {
                    struct sockaddr_in* sin = (struct sockaddr_in*)(&local_addr);
                    this->port = ntohs(sin->sin_port);
                    localIP.setIp(inet_ntoa(sin->sin_addr));
                    localIP.setPort(this->port);
                    cout<<"Local IP & port "<<localIP.getIp()<<" : "<<localIP.getPort()<<endl;
                }
            }

            Message msg( 1, MessageType::REGISTER, "register");
            string ret = client.SyncSend(msg);
            //cout<<"Consumer Register ret : "<<ret<<endl;
            if(ret.find("Ack") != string::npos)
            {
                cout<<"注册成功"<<endl;
            }
            else
            {
                cout<<"注册失败, ret : "<<ret<<endl;
            }
        }
        catch (std::exception e) {
            cout<<"Connection Refuse . REGISTER"<<endl;
        }
    };
    thread thrd(f);
    thrd.join();
}

void Consumer::Register(IpNode ipNode, int clientPort)
{
    auto f = [&](){
        cout<<"正在注册Consumer..."<<endl;
        try{
            //Client client(ipNode.getIp(), ipNode.getPort(), clientPort);
            client = new Client(ipNode.getIp(), ipNode.getPort(), clientPort);
            fd = client->getClientFd();
            IpNode localIP("",-1);
            if(fd < 0)
            {
                cout<<"连接失败"<<endl;
            }
            else
            {
                struct sockaddr local_addr;
                socklen_t len = sizeof(sockaddr);

                if(getsockname(fd, &local_addr, &len) == 0)
                {
                    struct sockaddr_in* sin = (struct sockaddr_in*)(&local_addr);
                    this->port = ntohs(sin->sin_port);
                    localIP.setIp(inet_ntoa(sin->sin_addr));
                    localIP.setPort(this->port);
                    cout<<"Local IP & port "<<localIP.getIp()<<" : "<<localIP.getPort()<<endl;
                }
            }

            //Topic t;
            Message msg( 1, MessageType::REGISTER, "register");
            string ret = client->SyncSend(msg);
            //cout<<"Consumer Register ret : "<<ret<<endl;
            if(ret.find("Ack") != string::npos)
            {
                cout<<"注册成功"<<endl;
            }
            else
            {
                cout<<"注册失败, ret : "<<ret<<endl;
            }
        }
        catch (std::exception e) {
            cout<<"Connection Refuse . REGISTER"<<endl;
        }
    };
    thread thrd(f);
    thrd.join();
}

void Consumer::listening()
{
    auto f = [&](){
        int startNum = 0;
        checkMsg.clear();
        int START = 0;
        //int All = 0;
        while(true)
        {
            if(read(fd, buf, sizeof(buf)))
            {
                if(START == 0) // 开始时间
                {
                    t_start = time(0);
                    START++;
                }
                string str = buf;
                memset(buf, 0, sizeof(buf));

                //test
                //cout<<"Consumer"<<count<<", the "<<T++<<"th time, "<<str<<endl;

                Message *msg = SerializeUtil::anti_serialize_to_Message(str);

                string reply = "Ack " + to_string(msg->getNum()+1);
                client->Send(reply);
                //cout<<"Consumer"<<count<<" reply Sent : "<<reply<<endl;

                ConsumerMessage cmsg(*msg);
                //cout<<"consumer"<<count<<", T =  "<<T++<<"th time, cmsg : num = "<<cmsg.getNum()<<" priority = "<<cmsg.getPriority()<<" message = "<<cmsg.getMsg()<<endl;

                if(checkMsg.count(cmsg) == 0)
                {
                    //cout<<"Not existed. consumer"<<count<<" "<<"cmsg : num = "<<cmsg.getNum()<<" priority = "<<cmsg.getPriority()<<" message = "<<cmsg.getMsg()<<endl;
                    checkMsg.insert(cmsg);
                    priorityQueue.push(cmsg);
                }
                else{
                    //cout<<"Existed. consumer"<<count<<" "<<"cmsg : num = "<<cmsg.getNum()<<" priority = "<<cmsg.getPriority()<<" message = "<<cmsg.getMsg()<<endl;
                }
                //cout<<"Consumer"<<count<<" Get, Message Num = "<<msg->getNum()<<" , Message = "<<msg->getMessage()<<endl;
                memset(buf, 0, sizeof(buf));
            }

            while(true)
            {
                ConsumerMessage consumerMessage = priorityQueue.top();
                //cout<<"consumer"<<count<<" "<<"priority top : num = "<<consumerMessage.getNum()<<" priority = "<<consumerMessage.getPriority()<<" message = "<<consumerMessage.getMsg()<<endl;
                int pos = consumerMessage.getMsg().find(" ");
                int Num = consumerMessage.getNum();
                if(pos != string::npos)
                {
                    Num = stoi(consumerMessage.getMsg().substr(pos));
                }
                //cout<<"Num:"<<Num<<endl;
                if(Num == startNum)
                {
                    //cout<<"T:"<<T<<endl;
                    priorityQueue.pop();
                    startNum++;
                    cout<<"Consumer"<<count<<" received message. Num : "<<consumerMessage.getNum()<<", Message : "<<consumerMessage.getMsg()<<endl;
                    //cout<<"ALL:"<<All++<<endl;
                    if(Num + 1 == TOTAL_MESSAGE_NUM)
                    {
                        t_end = time(0);
                        cout<<"Consumer"<<count<<" END. Total Time = "<<difftime(t_end, t_start)<<" s"<<endl;
                        cout<<"Consumer"<<count<<" END Time = "<<t_end<<" s"<<endl;
                    }

                }
                else
                {
                    break;
                }
            }

        }
        close(fd);
    };
    thread thrd(f);
}

time_t Consumer::getTend()
{
    return t_end;
}

/*void Consumer::EventHandle(int epfd, struct epoll_event ev)
{
    if(ev.events & EPOLLIN) {
        char *buf = (char *) malloc(BufferSize);
        memset(buf, 0, BufferSize);
        int count = 0;
        int n = 0;
        //repeatly read
        while (1) {
            n = read(fd, (buf + count), 10);
            if (n > 0) {
                count += n;
            } else if (n == 0) {
                break;
            } else if (n < 0 && errno == EAGAIN) {
                //cout<<strerror(errno)<<endl;
                break;
            } else {
                cout << "Read Error:" << strerror(errno) << endl;
                exit(-1);

            }
        }
        if(count == 0){
            cout<<"连接已断开"<<endl;
            if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) != 0){
                cout<<"Epoll CTL Delete Error:"<<strerror(errno)<<endl;
                exit(-1);
            }
            close(fd);
            return;
        }

        cout<<buf<<endl;
    }
}*/

