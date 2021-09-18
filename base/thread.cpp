//
// Created by Administrator on 2021/9/18.
//
#include <thread>
#include <iostream>

void sayHello(const std::string &name)
{
    std::cout << "[pid:" << std::this_thread::get_id() << "] hello " << name << std::endl;
}

void threadTest()
{
    std::string name = "lsm";
    std::thread t(sayHello, name);
    t.join();
}