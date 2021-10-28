set(src
        main.cpp

        http/http_request.cpp
        http/http_response.cpp
        common/common.cpp
        common/strings.cpp
        common/banner.cpp

        core/config.cpp
        core/setproctitle.cpp

        log/log.cpp
        log/log_printf.cpp
        )

add_executable(nginx ${src})