//
// Created by ironzhou on 2020/8/3.
//

//#include "../../include/Utils/SequenceUtil.h"
#include "../../include/ALL.h"

int SequenceUtil::getSequence()
{
    boost::lock_guard<boost::mutex> lock(sequenceMutex);
    return count++;
}

/*int main()
{
    SequenceUtil su;
    std::cout<<su.getSequence()<<std::endl;
    std::cout<<su.getSequence()<<std::endl;
    return 0;
}*/