#include <iostream>
#include <thread>
#include "BrokerServer.cpp"
#include "include/config.h"

int main() {
    unsigned int n = std::thread::hardware_concurrency();
    if (n == 0) n = 1;
    if (n >=MAX_WORKER) n = MAX_WORKER;
    BrokerServer broker;
    broker.start(n);

    return 0;
}