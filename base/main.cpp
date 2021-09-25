//
// Created by Administrator on 2021/9/17.
//

#include "main.h"

int main()
{
    void* p=::operator new(512);
    initializerTest();

    structBindTest();

    smartPointTest();

    threadTest();
    return 0;
}