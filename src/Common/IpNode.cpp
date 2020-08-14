//
// Created by ironzhou on 2020/7/24.
//
//#include "../../include/Common/IpNode.h"
#include "../../include/ALL.h"

/*template<class Archive>
void IpNode::serialize(Archive &ar, const unsigned int version)
{
    ar & ip;
    ar & port;
}*/

IpNode::IpNode() {}

IpNode::IpNode(string i, int p):ip(i), port(p) {}

string IpNode::getIp()
{
    return ip;
}

void IpNode::setIp(string s)
{
    this->ip = s;
}

int IpNode::getPort()
{
    return port;
}

void IpNode::setPort(int p)
{
    this->port = p;
}

bool IpNode::operator==(const IpNode &compareTo) const
{
    if(this->ip == compareTo.ip && this->port == compareTo.port)
    {
        return true;
    }
    return false;
}

bool IpNode::operator<(const IpNode &compareTo) const
{
    if(this->ip == compareTo.ip)
        return this->port < compareTo.port;
    else
        return this->ip < compareTo.ip;
}
