//
// Created by Administrator on 2021/10/18.
//

#include "frame.h"

frame::frame(int fd)
{
    char buf[8] = {0};
    ssize_t n = read(fd, buf, 2);
    if (n != 2)
    {
        this->bad_message = true;
        return;
    }
    this->byte1_ = buf[0];
    this->byte2_ = buf[1];
    if (this->mask() == 1 && read(fd, this->masking_key_, sizeof(this->masking_key_)) != 4)
    {
        this->bad_message = true;
        return;
    }
    if (this->length() == 126 && read(fd, buf, 2) == 2)
    {
        this->payload_length = frame::char_2_int64(buf);
    } else if (this->length() == 127 && read(fd, buf, sizeof(buf)) == 8)
    {
        this->payload_length = frame::char_2_int64(buf);
    } else if (this->length() >= 126)
    {
        this->bad_message = true;
        return;
    }
}

bool frame::is_bad_message()
{
    return this->bad_message;
}

char frame::op_code()
{
    return this->byte1_ & 0x0f;
}

char frame::fin()
{
    return this->byte1_ >> 7;
}

char frame::mask()
{
    return this->byte2_ >> 7;
}

int64_t frame::length()
{
    int8_t len = this->byte2_ & 0b01111111;
    if (len < 126)
    {
        return len;
    } else
    {
        return payload_length;
    }
}

int64_t frame::char_2_int64(const char *buf)
{
    int64_t value = 0;

    for (int8_t i = 7; i >= 0; i--)
    {
        value <<= 8;
        value |= buf[i];
    }
    return value;
}
