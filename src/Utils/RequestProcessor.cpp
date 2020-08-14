//
// Created by ironzhou on 2020/8/4.
//
//#include "../../include/Utils/RequestProcessor.h"

#include "../../include/ALL.h"

RequestProcessor::RequestProcessor(){}

void RequestProcessor::processRequest(int epfd, struct epoll_event ev)
{
    int fd = ev.data.fd;
    char* buf = (char*)malloc(BufferSize);
    memset(buf, 0, BufferSize);
    int count = 0;
    int n = 0;
    //repeatly read
    while(1){
        n = read(fd, (buf+count), 10);
        if(n > 0){
            count += n;
        }
        else if(n == 0){
            break;
        }
        else if(n < 0 && errno == EAGAIN){
            //cout<<strerror(errno)<<endl;
            break;
        }
        else{
            cout<<"Read Error:"<<strerror(errno)<<endl;
            exit(-1);
        }
    }
    if(count == 0){
        if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev) != 0){
            cout<<"Epoll CTL Delete Error:"<<strerror(errno)<<endl;
            exit(-1);
        }
        close(fd);
        return;
    }

    //set to write
    struct Epoll_Data* ed = (struct Epoll_Data*)malloc(sizeof(struct Epoll_Data));
    ed->data = buf;
    ed->fd = fd;
    ev.data.ptr = ed;
    ev.events = EPOLLOUT | EPOLLET;
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) != 0) {
        cout<<"Epoll Read-Write state Changing Error:"<<strerror(errno)<<endl;
        exit(-1);
    }

    return;
}
