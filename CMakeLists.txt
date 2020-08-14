cmake_minimum_required(VERSION 3.16)
project(MyMQ)

#find_package(TBB COMPONENTS tbbmalloc tbbmalloc_proxy tbb_preview)
find_package(RapidJSON)

add_executable(BrokerTest
        src/Test/BrokerTest
        include/ALL
        src/Common/IpNode
        src/Common/Topic
        src/Common/Message
        src/Broker/MyMutex
        src/Broker/MyQueue
        src/Broker/Filter
        src/Broker/LoadBalancer
        src/Utils/SerializeUtil
        src/Utils/PersistenceUtil
        src/Utils/Client
        src/Broker/Broker
        src/Utils/RequestProcessor
        src/Utils/ResponseProcessor
        src/Broker/Slave
        src/Utils/Server
        )

add_executable(ConsumerTest
        src/Test/ConsumerTest
        include/ALL
        src/Common/IpNode
        src/Common/Topic
        src/Common/Message
        src/Broker/MyMutex
        src/Broker/MyQueue
        src/Broker/Filter
        src/Broker/LoadBalancer
        src/Utils/SerializeUtil
        src/Utils/Client
        src/Utils/PersistenceUtil
        src/Broker/Broker
        src/Utils/RequestProcessor
        src/Utils/ResponseProcessor
        src/Broker/Slave
        src/Utils/Server
        src/Consumer/ConsumerMessage
        src/Consumer/Consumer
        )

add_executable(ProducerTest
        src/Test/ProducerTest
        include/ALL
        src/Common/IpNode
        src/Common/Topic
        src/Common/Message
        src/Broker/MyMutex
        src/Broker/MyQueue
        src/Broker/Filter
        src/Broker/LoadBalancer
        src/Utils/SerializeUtil
        src/Utils/SequenceUtil
        src/Utils/Client
        src/Utils/PersistenceUtil
        src/Broker/Broker
        src/Utils/RequestProcessor
        src/Utils/ResponseProcessor
        src/Broker/Slave
        src/Utils/Server
        src/Producer/Producer
        )

add_executable(TopicRegisterTest
        src/Test/TopicRegisterTest
        include/ALL
        src/Common/IpNode
        src/Common/Topic
        src/Common/Message
        src/Broker/MyMutex
        src/Broker/MyQueue
        src/Broker/Filter
        src/Broker/LoadBalancer
        src/Utils/SerializeUtil
        src/Utils/Client
        src/Utils/PersistenceUtil
        src/Broker/Broker
        src/Utils/RequestProcessor
        src/Utils/ResponseProcessor
        src/Broker/Slave
        src/Utils/Server
        src/Consumer/ConsumerMessage
        src/Consumer/Consumer
        src/Utils/TopicRegister
        )

add_executable(SlaveTest
        src/Test/SlaveTest
        include/ALL
        src/Common/IpNode
        src/Common/Topic
        src/Common/Message
        src/Broker/MyMutex
        src/Broker/MyQueue
        src/Broker/Filter
        src/Broker/LoadBalancer
        src/Utils/SerializeUtil
        src/Utils/PersistenceUtil
        src/Utils/Client
        src/Broker/Broker
        src/Utils/RequestProcessor
        src/Utils/ResponseProcessor
        src/Broker/Slave
        src/Utils/Server
        )

add_executable(NameServerTest
        src/Test/NameServerTest
        include/ALL
        src/Common/IpNode
        src/Common/Topic
        src/Common/Message
        src/Broker/MyMutex
        src/Broker/MyQueue
        src/Broker/Filter
        src/Broker/LoadBalancer
        src/Utils/SerializeUtil
        src/Utils/PersistenceUtil
        src/Utils/Client
        src/Broker/Broker
        src/Utils/NameServer
        src/Utils/RequestProcessor
        src/Utils/ResponseProcessor
        src/Broker/Slave
        src/Utils/Server
        )

#add_executable(JsonTest
#        src/Test/JsonTest
#        )

#add_executable(RapidJsonTest
#        src/Test/rapidJsonTest.cpp
#        )

target_link_libraries(BrokerTest json boost_serialization boost_thread boost_system tbb)
target_link_libraries(ConsumerTest json boost_serialization boost_thread boost_system tbb)
target_link_libraries(ProducerTest json boost_serialization boost_thread boost_system tbb)
target_link_libraries(TopicRegisterTest json boost_serialization boost_thread boost_system tbb)
target_link_libraries(SlaveTest json boost_serialization boost_thread boost_system tbb)
target_link_libraries(NameServerTest json boost_serialization boost_thread boost_system tbb)
#target_link_libraries(JsonTest json boost_serialization boost_thread boost_system tbb)
#target_link_libraries(RapidJsonTest boost_serialization boost_thread boost_system tbb)