//
// Created by ironzhou on 2020/8/12.
//
#include "../../include/ALL.h"

int main()
{
    boost::thread_group threadGroup;
    set<int> NameServers; NameServers.clear();
    NameServers.insert(12345);
    auto f = [&](int p)
    {
        NameServer nameServer(p);
    };
    for(auto it : NameServers)
    {
        threadGroup.create_thread(boost::bind<void>(f,it));
    }

    threadGroup.join_all();

    pause();
    return 0;
}
