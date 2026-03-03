#include <iostream>
#include <thread>
#include "BrokerServer.cpp"
#include "include/config.h"

int main() {
    unsigned int n = std::thread::hardware_concurrency();

    std::cout<<"CPU cores: "<<n<<std::endl;

    if (n == 0) n = 1;
    if (n >=MAX_WORKER) n = MAX_WORKER;

    std::cout<<"Max worker:  "<<n<<std::endl;

    BrokerServer broker;
    broker.start(n);

    return 0;
}