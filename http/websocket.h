//
// Created by Administrator on 2021/10/18.
//

#ifndef NETWORK_WEBSOCKET_H
#define NETWORK_WEBSOCKET_H

#include <thread>
#include <string>
#include <utility>
#include <xtcp.h>
#include <sha1.h>
#include <base64.h>
#include "http_request.h"
#include "http_response.h"
#include "frame.h"

constexpr const char *upgrade_key = "Upgrade";

constexpr const char *upgrade_value = "websocket";

constexpr const char *connection_key = "Connection";

constexpr const char *connection_value = "Upgrade";

constexpr const char *sec_websocket_version_key = "Sec-WebSocket-Version";

constexpr const char *sec_websocket_version_value = "13";

constexpr const char *sec_websocket_key_key = "Sec-WebSocket-Key";

constexpr const char *sec_websocket_accept_key = "Sec-WebSocket-Accept";

constexpr const char *ws_magic_key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

class websocket_server
{
    using String = std::string;

private:
    int port;

    String path;

public:
    websocket_server(int port) : port(port)
    {}

    websocket_server(String path) : path(std::move(path))
    {}

    websocket_server(int port, String path) : port(port), path(std::move(path))
    {}

    ~websocket_server();

    void run();

    int upgrade(int fd);

    String parse_sec_websocket_key(const String &sec_websocket_key_key);

    int read(int fd);
};


#endif //NETWORK_WEBSOCKET_H
