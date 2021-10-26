set(src
        main.cpp
        log/log.cpp
        http/http_request.cpp
        http/http_response.cpp
        common/common.cpp
        common/strings.cpp
        common/banner.cpp

        core/config.cpp
        )

add_executable(nginx ${src})