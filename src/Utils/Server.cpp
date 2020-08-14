//
// Created by ironzhou on 2020/7/27.
//
//

//#include "../../include/Utils/Server.h"

#include "../../include/ALL.h"

/*


Server::Server(int port, RequestProcessor requestProcessor, ResponseProcessor responseProcessor)
        :listen_port(port),
        _acceptor(_ios),
        requestProcessor(requestProcessor),
        responseProcessor(responseProcessor)
{
    init(port);
}

Server::Server(int port, RequestProcessor requestProcessor, ResponseProcessor responseProcessor, Broker broker)
        :listen_port(port),
         _acceptor(_ios),
         requestProcessor(requestProcessor),
         responseProcessor(responseProcessor),
         broker(broker)
{
    init(port);
}

Server::Server(int port, RequestProcessor requestProcessor, ResponseProcessor responseProcessor, Slave slave)
        :listen_port(port),
         _acceptor(_ios),
         requestProcessor(requestProcessor),
         responseProcessor(responseProcessor),
         slave(slave)
{
    init(port);
}



void Server::init(int port)
{
    // open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), listen_port);
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    // get our first connection ready


    // pump the first async accept into the loop


    // start up the event loop for the service
    _runner = boost::thread(boost::bind(&asio::io_service::run, &_ios));

}

void Server::Stop()
{
    _ios.stop();

    // let the thread exit
    _runner.join();
}*/

Server::Server(int port, RequestProcessor requestProcessor, ResponseProcessor responseProcessor, Broker & broker, Slave & slave, NameServer* nameServer1)
        :requestProcessor(requestProcessor),
         responseProcessor(responseProcessor),
         broker(broker),
         slave(slave),
         nameServer(nameServer1)
{
    init(port);
}

Server::Server(int port, RequestProcessor requestProcessor, ResponseProcessor responseProcessor, Broker & broker, Slave & slave)
        :requestProcessor(requestProcessor),
         responseProcessor(responseProcessor),
         broker(broker),
         slave(slave)
{
    init(port);
}

void Server::init(int port)
{
    cout<<"Server 已启动, 在 "<<port<<" 端口监听"<<endl;

    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    //serverAddr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, ServerIP, &serverAddr.sin_addr); // change IP(Decimal to binary)
    serverFd = socket(AF_INET, SOCK_STREAM, 0); // tcp
    if(serverFd < 0)
    {
        close(serverFd);
        cout<<"Server Socket Error : "<<strerror(errno)<<endl;
        exit(-1);
    }

    //Bind
    if(::bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0)
    {
        close(serverFd);
        cout<<"Server Binding Error:"<<strerror(errno)<<endl;
        exit(-1);
    }

    // Listen
    if(listen(serverFd, ListenBacklog) != 0)
    { // how to decide backlog? in linux the maximum is 128
        close(serverFd);
        cout<<"Listening Error:"<<strerror(errno)<<endl;
        exit(-1);
    }

    // Create epoll
    epollFd = epoll_create1(EPOLL_CLOEXEC);//or 0
    if(epollFd < 0)
    {
        close(serverFd);close(epollFd);
        cout<<"Epoll Creating Error:"<<strerror(errno)<<endl;
        exit(-1);
    }
    addFd(epollFd, serverFd);
    start(port);
}

void Server::addFd(int epollFd, int fd)
{
    struct epoll_event epollEvent;
    epollEvent.data.fd = fd;
    epollEvent.events = EPOLLIN | EPOLLET; // use ET mode; multiple threads use EPOLLONESHOT
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &epollEvent) != 0)
    {   // is this necessary?
        close(fd);close(epollFd);
        cout<<"Epoll CTL Error : "<<strerror(errno)<<endl;
        exit(-1);
    }
    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK) != 0)
    {   // is this "if" necessary?
        // set non-blocking I/O
        close(fd);close(epollFd);
        cout<<"Setting I/O Error : "<<strerror(errno)<<endl;
        exit(-1);
    }
}

void Server::start(int port)
{
    while(true)
    {
        int EpollEventNumber = epoll_wait(epollFd, epollEvents, EpollSize, -1);//-1:block/ 0:non-block
        if(EpollEventNumber < 0)
        {
            cout<<"Epoll Waiting Error"<<strerror(errno)<<endl;
            exit(-1);//continue break? exit?
            //continue;
        }
        for(int i = 0; i < EpollEventNumber ; i++)
        {
            // Accept
            if(epollEvents[i].data.fd == serverFd)
            {   // new connection
                socklen_t clientAddrLen = sizeof(struct sockaddr_in);
                // multiple connections at the same time
                while((clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen)) > 0)
                {
                    addFd(epollFd, clientFd);
                    cout<<"Client IP addr:("<<inet_ntoa(clientAddr.sin_addr)<<":"
                        <<ntohs(clientAddr.sin_port)<<"), ClientFd："<<clientFd<<endl;
                }
                if(clientFd < 0)
                {
                    if(errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
                    {
                        cout<<"Accept Error"<<endl;
                    }
                }
            }
            else{
                //cout<<"Handle Event"<<endl;
                EventHandle(epollFd, epollEvents[i]);
            }
        }
    }
}

void Server::EventHandle(int epfd, struct epoll_event ev)
{
    if(ev.events & EPOLLIN)
    {
        // 调用读操作工具类
        //cout<<"Handle Request"<<endl;
        //if(responseProcessor.getType() == 1)
        //    requestProcessor.processRequest(epfd, ev, 0);
        requestProcessor.processRequest(epfd, ev);
    }
    else if(ev.events & EPOLLOUT)
    {
        // 选择写操作工具
        if(responseProcessor.getType() == 0)
        {
            // brokerResponse
            //cout<<"Handle Broker Response"<<endl;
            responseProcessor.processResponse(epfd, ev, broker);

            /*test
            cout<<"responseProcessor.getType() == 0 GetAll Start"<<endl;
            broker.getAll();
            cout<<"responseProcessor.getType() == 0 GetAll Exit"<<endl;*/
        }
        else if(responseProcessor.getType() == 1)
        {
            // slaveResponse
            //cout<<"SlaveResponse"<<endl;
            responseProcessor.processResponse(epfd, ev, slave);
        }
        else if(responseProcessor.getType() == 2)
        {
            //NameServer
            responseProcessor.processResponse(epfd, ev, *nameServer);
        }
        else
        {
            // default
            responseProcessor.processResponse(epfd, ev);
        }
    }
    return ;
}

