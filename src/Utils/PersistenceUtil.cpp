//
// Created by ironzhou on 2020/8/11.
//
#include "../../include/ALL.h"

//using namespace rapidjson;

void PersistenceUtil::BrokerPersistence(Broker broker, string path)
{
    Json::Value root;
    root["count"] = Json::Value(broker.count);
    root["push_Time"] = Json::Value(broker.push_Time);
    root["hasSlave"] = Json::Value(broker.hasSlave);
    root["startPersistence"] = Json::Value(broker.startPersistence);
    root["pushMode"] = Json::Value(broker.pushMode);
    root["sync_Time"] = Json::Value(broker.sync_Time);
    root["reTry_Time"] = Json::Value(broker.reTry_Time);
    root["store_Time"] = Json::Value(broker.store_Time);
    // Filter 序列化后存储
    root["filter"] = Json::Value(SerializeUtil::serialize_Filter(broker.filter));
    // index
    for(auto it : broker.index)
    {
        root["index"].append(SerializeUtil::serialize_IpNode(it));
    }
    // slave
    for(auto it : broker.slave)
    {
        root["slave"].append(SerializeUtil::serialize_IpNode(it));
    }
    // clients
    Json::Value sonClient;
    for(auto it : broker.clients)
    {
        sonClient[SerializeUtil::serialize_IpNode(it.first)] = Json::Value(it.second);
    }
    root["clients"] = Json::Value(sonClient);
    // deadQueue
    Json::Value sonDeadQueue;
    for(auto it : broker.deadQueue)
    {
        sonDeadQueue[it.first] = Json::Value(SerializeUtil::serialize_Message(it.second));
    }
    root["deadQueue"] = Json::Value(sonDeadQueue);
    // TopicQueueMap
    Json::Value sonTopicQueue;
    for(auto it : broker.topicQueueMap)
    {
        for(auto that : it.second)
        {
            sonTopicQueue[SerializeUtil::serialize_Topic(it.first)].append(that);
        }
    }
    root["topicQueueMap"] = Json::Value(sonTopicQueue);
    // queueList
    Json::Value sonQueueList;
    for(auto it : broker.queueList)
    {
        sonQueueList[it.first] = Json::Value(SerializeUtil::serialize_MyQueue(it.second));
    }
    root["queueList"] = Json::Value(sonQueueList);

    Json::StyledWriter sw;
    sw.write(root);

    ofstream os;
    path += "BrokerPersistenceJson.json";
    os.open(path, std::ios::out);
    if(!os.is_open())
    {
        cout << "error：can not find or create the file which named \" "<< path << " \" " << endl;
    }
    os << sw.write(root);
    os.close();
}

Broker PersistenceUtil::BrokerRecovery(string path)
{
    Json::Reader reader;
    Json::Value root;

    Broker broker;

    path += "BrokerPersistenceJson.json";
    ifstream in(path, ios::in | ios::binary);

    if(!in.is_open())
    {
        cout << "Error opening file \" "<< path << " \" " << endl;
        return broker;
    }

    if(reader.parse(in, root))
    {
        // 普通成员恢复
        if(root.isMember("count"))
        {
            broker.count = root["count"].asInt();
        }
        if(root.isMember("push_Time"))
        {
            broker.setPushTime(root["push_Time"].asInt());
        }
        if(root.isMember("hasSlave"))
        {
            broker.hasSlave = root["hasSlave"].asBool();
        }
        if(root.isMember("startPersistence"))
        {
            broker.startPersistence = root["startPersistence"].asBool();
        }
        if(root.isMember("pushMode"))
        {
            broker.pushMode = root["pushMode"].asBool();
        }
        if(root.isMember("sync_Time"))
        {
            broker.sync_Time = root["sync_Time"].asInt();
        }
        if(root.isMember("reTry_Time"))
        {
            broker.reTry_Time = root["reTry_Time"].asInt();
        }
        if(root.isMember("store_Time"))
        {
            broker.store_Time = root["store_Time"].asInt();
        }

        // Filter恢复
        if(root.isMember("filter"))
        {
            broker.filter = SerializeUtil::anti_serialize_to_Filter(root["filter"].asString());
        }

        // index恢复
        if(root.isMember("index"))
        {
            set<IpNode> ips; ips.clear();
            for(unsigned int i = 0 ; i < root["index"].size() ; i++)
            {
                ips.insert(SerializeUtil::anti_serialize_to_IpNode(root["index"][i].asString()));
            }
            broker.index = ips;
        }

        // slave恢复
        if(root.isMember("slave"))
        {
            set<IpNode> slaves; slaves.clear();
            for(unsigned int i = 0 ; i < root["slave"].size() ; i++)
            {
                slaves.insert(SerializeUtil::anti_serialize_to_IpNode(root["slave"][i].asString()));
            }
            broker.slave = slaves;
        }

        // clients恢复
        if(root.isMember("clients"))
        {
            map<IpNode, int> clients; clients.clear();
            Json::Value::Members mem = root["clients"].getMemberNames();
            for(auto it = mem.begin() ; it != mem.end() ; it++)
            {
                clients.insert(make_pair(SerializeUtil::anti_serialize_to_IpNode(*it), root["clients"][*it].asInt()));
            }
            broker.clients = clients;
        }

        // deadQueue恢复
        if(root.isMember("deadQueue"))
        {
            map<int, Message> deadQueue; deadQueue.clear();
            Json::Value::Members mem = root["deadQueue"].getMemberNames();
            for(auto it = mem.begin() ; it != mem.end() ; it++)
            {
                deadQueue.insert(make_pair(stoi(*it),*SerializeUtil::anti_serialize_to_Message(root["deadQueue"][*it].asString())));
            }
            broker.deadQueue = deadQueue;
        }

        // topicQueueMap恢复
        if(root.isMember("topicQueueMap"))
        {
            map<Topic, set<string> > topicQueueMap; topicQueueMap.clear();
            Json::Value::Members mem = root["topicQueueMap"].getMemberNames();
            for(auto it = mem.begin() ; it != mem.end() ; it++)
            {
                Topic tempTopic = SerializeUtil::anti_serialize_to_Topic(*it);
                for(unsigned int i = 0 ; i < root["topicQueueMap"][*it].size() ; i++)
                {
                    topicQueueMap[tempTopic].insert(root["topicQueueMap"][*it][i].asString());
                }
            }
            broker.topicQueueMap = topicQueueMap;
        }

        // queueList恢复
        if(root.isMember("queueList"))
        {
            concurrent_hash_map<string, MyQueue> queueList; queueList.clear();
            Json::Value::Members mem = root["queueList"].getMemberNames();
            for(auto it = mem.begin() ; it != mem.end() ; it++)
            {
                queueList.insert(make_pair(*it, SerializeUtil::anti_serialize_to_MyQueue(root["queueList"][*it].asString())));
            }
            broker.queueList = queueList;
        }
    }
    else
    {
        cout << "Parse Error!" << endl;
    }


    return broker;

}

/*rapidjson::Document PersistenceUtil::persistentConsumer(set<IpNode> consumerAddress, string path)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType & allocator = document.GetAllocator();
    rapidjson::Value IpNodeArray(kArrayType);
    rapidjson::Value str_value(kStringType);
    for(auto it : consumerAddress)
    {
        string temp = SerializeUtil::serialize_IpNode(it);
        str_value.SetString(temp.c_str(), temp.size());
        IpNodeArray.PushBack(str_value, allocator);
    }
    document.AddMember("Consumer", IpNodeArray, allocator);

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    document.Accept(writer);

    string data = strBuf.GetString();
    //cout << data << endl;
    ofstream os;
    path += "ConsumerPersistence.json";
    os.open(path, std::ios::out);
    if(!os.is_open())
    {
        cout << "error：can not find or create the file which named \" "<< path << " \" " << endl;
    }
    os << data;
    os.close();


    return document;
}

bool PersistenceUtil::Export(rapidjson::Document document, string path)
{
    string data = "\"Consumer\":[]";
    rapidjson::Document d;
    if(!d.Parse(data.data()).HasParseError())
    {
        if(d.HasMember("Consumer") && d["Consumer"].IsArray())
        {
            const rapidjson::Value  & array = d["Consumer"];
            rapidjson::size_t len = array.Size();
            for(rapidjson::size_t i = 0 ; i < len ; i++)
            {
                if(array[i].IsString())
                {
                    IpNode ipNode = SerializeUtil::anti_serialize_to_IpNode(array[i].GetString());
                    cout<<"ipNode = "<<ipNode.getIp()<<" : "<<ipNode.getPort()<<endl;
                }
                else
                {
                    cout<<"Not String"<<endl;
                }
            }
        }
        else
        {
            cout<<"Has Member Consumer ? "<<d.HasMember("Consumer")<<endl;
            cout<<"No Member Or Not Array"<<endl;
        }
    }
    else
    {
      cout<<"Parse Error"<<endl;
    }

    return true;
}*/