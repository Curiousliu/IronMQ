//
// Created by ironzhou on 2020/7/24.
//

//#include "../../include/Common/Message.h"
#include "../../include/ALL.h"

Message::Message() {}

Message::Message(int num, int type) :num(num), type(type){}

Message::Message(int num, Topic topic1, string s)
        :num(num),topic(topic1),message(s),type(MessageType::REPLY_EXPECTED){}

Message::Message(int num, int type, string s)
:num(num), type(type), message(s){}

Message::Message(int num, int type, Topic topic1, string s)
        : num(num), type(type), topic(topic1), message(s){}

/*Message::Message(const Message &msg)
{
    num = msg.num;
    type = msg.type;
    message = msg.message;
    priority = msg.priority;
    topic = msg.topic;
}*/

string Message::getMessage()
{
    return message;
}

int Message::getType()
{
    return type;
}

void Message::setType(int type)
{
    if(type >= MessageType::ONE_WAY && type <= MessageType::PULL)
        this->type = type;
    else
        this->type = MessageType::REPLY_EXPECTED;
}

Topic Message::getTopic()
{
    return topic;
}

void Message::setTopic(Topic top)
{
    this->topic = top;
}

int Message::getNum()
{
    return num;
}

void Message::setNum(int num)
{
    this->num = num;
}

bool Message::operator<(const Message &m) const
{
    if(m.priority == priority)
    {
        if(m.num == num)
        {
            return m.topic < topic; // topic随意，这边是小顶堆
        }
        return m.num < num;         // 消息序号是小顶堆
    }
    return priority < m.priority;   // 优先级是大顶堆
}

void Message::setPriority(int num)
{
    priority = num;
}

int Message::getPriority()
{
    return priority;
}