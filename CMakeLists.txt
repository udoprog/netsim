PROJECT(cavalieri)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(NETSIM_SRCS
    ${CMAKE_SOURCE_DIR}/src/server.cpp
    ${CMAKE_SOURCE_DIR}/src/netsim.cpp
    ${CMAKE_SOURCE_DIR}/src/io.cpp
    ${CMAKE_SOURCE_DIR}/src/messages.cpp
    )

SET(NETSIM_PRELOAD_SRCS
    ${CMAKE_SOURCE_DIR}/src/netsim-preload.cpp
    ${CMAKE_SOURCE_DIR}/src/simulator.cpp
    ${CMAKE_SOURCE_DIR}/src/socket_proxy.cpp
    ${CMAKE_SOURCE_DIR}/src/io.cpp
    ${CMAKE_SOURCE_DIR}/src/messages.cpp
    )

ADD_DEFINITIONS(-Wall -Wextra -std=c++11 -g)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/
    )

ADD_DEFINITIONS(
    -Wall -std=c++11 -g
    )

ADD_EXECUTABLE(
    netsim
    ${NETSIM_SRCS}
    )

ADD_LIBRARY(
    netsim-preload
    SHARED
    ${NETSIM_PRELOAD_SRCS}
    )

TARGET_LINK_LIBRARIES(
    netsim-preload
    dl
    pthread
    msgpack
    )
