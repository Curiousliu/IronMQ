//
// Created by ironzhou on 2020/8/9.
//
#include "../../include/ALL.h"

ConsumerMessage::ConsumerMessage() {}

ConsumerMessage::ConsumerMessage(Message msg)
{
    num = msg.getNum();
    //topic = msg.getTopic().getTopicName();
    priority = msg.getPriority();
    this->msg = msg.getMessage();
}

bool ConsumerMessage::operator<(const ConsumerMessage cmsg) const
{
    if(priority == cmsg.priority)
    {
        if(num == cmsg.num)
        {
            return msg > cmsg.msg;
        }
        return num > cmsg.num;
    }
    return priority > cmsg.priority; // 小顶堆
}

int ConsumerMessage::getNum() {return num;}

int ConsumerMessage::getPriority() {return priority;}

//string ConsumerMessage::getTopic() {return topic;}

string ConsumerMessage::getMsg() {return msg;}

void ConsumerMessage::setNum(int Num)
{
    num = Num;
}