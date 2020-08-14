//
// Created by ironzhou on 2020/7/30.
//

//#include "../../include/Broker/MyMutex.h"
#include "../../include/ALL.h"

MyMutex::MyMutex()
{
    mutex = new boost::mutex();
}

MyMutex::~MyMutex()
{
    delete mutex;
}

MyMutex::MyMutex(const MyMutex &mx)
{
    mutex = new boost::mutex;
}

MyMutex & MyMutex::operator=(const MyMutex &mx)
{
    return *this;
}

