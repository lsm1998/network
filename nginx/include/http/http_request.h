//
// Created by 刘时明 on 2021/10/4.
//

#ifndef NETWORK_HTTP_COMMON_H
#define NETWORK_HTTP_COMMON_H

#include <string>
#include <map>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "common.h"

constexpr int MAX_BUF_SIZE = 1024 * 4;

class http_request
{
    using String = std::string;

    using http_header = std::map<String, String>;

    using form_map = std::map<String, std::vector<String>>;

private:
    bool bad_request;

    String method;

    String path;

    String version;

    char *body;

    http_header header{};

    form_map query_map{};

    form_map form{};

    int sock_fd;

private:
    ssize_t read_line(void *buf, ssize_t max_line);

    ssize_t readn(void *buf, ssize_t count) const;

    ssize_t recv_peek(void *buf, ssize_t len) const;

    int parse_line(const String &line);

    int parse_header(const std::vector<String> &list);

    int parse_body();

    int parse_query(form_map *map, const String &query_str);

public:
    explicit http_request(int fd);

    ~http_request();

    [[nodiscard]] String query(const String& key) const;

    [[nodiscard]] String get_content_type() const;

    [[nodiscard]] String get_path() const;

    [[nodiscard]] bool is_bad_request() const;

    [[nodiscard]] char *get_body() const;

    [[nodiscard]] http_header get_header() const;

    [[nodiscard]] String get_header(const String &key) const;

    [[nodiscard]] String get_method() const;

    [[nodiscard]] String get_version() const;
};

#endif //NETWORK_HTTP_COMMON_H
