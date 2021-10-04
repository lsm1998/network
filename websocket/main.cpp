//
// Created by Administrator on 2021/9/21.
//
#include "network_interface.h"

int main()
{
    auto net = Network_Interface::get_share_network_interface();
    net->run();
    return 0;
}