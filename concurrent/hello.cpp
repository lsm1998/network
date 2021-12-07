//
// Created by 180PT73 on 2021/12/3.
//
#include <iostream>
#include <thread>

void sya_hello(int i)
{
    printf("sya_hello i 地址=%p \n", &i);
    std::cout << "hello world! for:" << i << std::endl;
}

int main()
{
    constexpr int SIZE = 100;
    std::thread threads[SIZE] = {};
    for (int i = 0; i < SIZE; ++i)
    {
        // printf("main i 地址=%p，值=%d \n", &i, i);
        threads[i] = std::thread(sya_hello, i);
    }
    for (auto &thread: threads)
    {
        thread.join();
    }
    return 0;
}