cmake_minimum_required(VERSION 3.22)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(MyWebServer
        buffer.cpp
        buffer.h
        epoller.cpp
        epoller.h
        httpconn.cpp
        httpconn.h
        httprequest.cpp
        httprequest.h
        httpresponse.cpp
        httpresponse.h
        main.cpp
        sqlconnRAII.cpp
        sqlconnRAII.h
        threadpool.h
        timer.cpp
        timer.h
        webserver.cpp
        webserver.h log.h log.h log.cpp)
