set(src
        main.cpp
        log/log.cpp
        http/http_request.cpp
        http/http_response.cpp
        common/common.cpp
        )

add_executable(nginx ${src})