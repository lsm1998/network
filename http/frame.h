//
// Created by Administrator on 2021/10/18.
//

#ifndef NETWORK_FRAME_H
#define NETWORK_FRAME_H

#include <cstdint>
#include <unistd.h>

class frame
{
private:
    char byte1_;
    char byte2_;
    char masking_key_[4];

    int64_t payload_length;

    char *body;

    bool bad_message;

private:
    static int64_t char_2_int64(const char* buf);

public:
    frame(int fd);

    bool is_bad_message();

    char fin();

    char op_code();

    char mask();

    int64_t length();
};


#endif //NETWORK_FRAME_H
