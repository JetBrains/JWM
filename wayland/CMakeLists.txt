cmake_minimum_required(VERSION 3.16)
# prefer the newer GL library (GLVND)
cmake_policy(SET CMP0072 NEW)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FindWaylandProtocols)
include(FindWaylandScanner)
find_package(WaylandProtocols 1.25)
set_package_properties(WaylandProtocols PROPERTIES
    TYPE REQUIRED
)
if (NOT WaylandProtocols_FOUND) 
    message(FATAL_ERROR "No protocols installed")
endif()
if (NOT WaylandScanner_FOUND)
    message(FATAL_ERROR "No wayland-scanner")
endif()
project(jwm LANGUAGES CXX)
project(protocols LANGUAGES C)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT JWM_ARCH)
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(JWM_ARCH "arm64")
    else()
        set(JWM_ARCH "x64")
    endif()
endif()

file(GLOB SOURCES_CXX ${CMAKE_CURRENT_LIST_DIR}/../shared/cc/*.cc 
    ${CMAKE_CURRENT_LIST_DIR}/../linux/cc/*.cc
    ${CMAKE_CURRENT_LIST_DIR}/cc/*.cc )
file(GLOB SOURCES_CXX_IMPL ${CMAKE_CURRENT_LIST_DIR}/../shared/cc/impl/*.cc)
add_library(jwm SHARED ${SOURCES_OBJC} ${SOURCES_CXX} ${SOURCES_CXX_IMPL})
ecm_add_wayland_client_protocol(PROTOCOLS_SOURCE 
    PROTOCOL "${WaylandProtocols_DATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
    BASENAME pointer-constraints-unstable-v1
)
ecm_add_wayland_client_protocol(PROTOCOLS_SOURCE
    PROTOCOL "${WaylandProtocols_DATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml"
    BASENAME relative-pointer-unstable-v1
)
ecm_add_wayland_client_protocol(PROTOCOLS_SOURCE
    PROTOCOL "${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml"
    BASENAME xdg-shell
)
ecm_add_wayland_client_protocol(PROTOCOLS_SOURCE
    PROTOCOL "${WaylandProtocols_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
    BASENAME xdg-decoration-unstable-v1
)
ecm_add_wayland_client_protocol(PROTOCOLS_SOURCE
    PROTOCOL "${WaylandProtocols_DATADIR}/stable/viewporter/viewporter.xml"
    BASENAME viewporter
)
ecm_add_wayland_client_protocol(PROTOCOLS_SOURCE
    PROTOCOL "${WaylandProtocols_DATADIR}/staging/xdg-activation/xdg-activation-v1.xml"
    BASENAME xdg-activation-v1
    )
add_library(protocols STATIC ${PROTOCOLS_SOURCE})
find_library(WAYLAND_CLIENT_LIB wayland-client)
find_library(WAYLAND_CURSOR wayland-cursor)
find_library(XKBCOMMON xkbcommon)
find_library(EGL EGL)
find_library(WAYLAND_EGL wayland-egl)
find_package(OpenGL REQUIRED)
set(JAVA_HOME $ENV{JAVA_HOME})
if (NOT JAVA_HOME)
    file(GLOB JAVA_HOMES "/usr/lib/jvm/java-*")
    if (JAVA_HOMES)
        list(GET JAVA_HOMES 0 JAVA_HOME)
        message(STATUS "Java home found automatically at ${JAVA_HOME}. Set JAVA_HOME environment variable to override.")
    else()
        message(FATAL_ERROR "Java home not found! Please set JAVA_HOME environment variable.")
    endif()
endif()

target_include_directories(jwm PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../shared/cc ${CMAKE_CURRENT_LIST_DIR}/../linux/cc ${JAVA_HOME}/include ${JAVA_HOME}/include/linux 
    ${CMAKE_CURRENT_LIST_DIR}/build)
set_target_properties(jwm PROPERTIES OUTPUT_NAME "jwm_${JWM_ARCH}_wayland")

target_link_libraries(jwm PRIVATE ${WAYLAND_CLIENT_LIB}
    ${WAYLAND_CURSOR} ${XKBCOMMON})
target_link_libraries(jwm PRIVATE ${EGL} ${WAYLAND_EGL})
target_link_libraries(jwm PRIVATE OpenGL::GL)
target_link_libraries(jwm PRIVATE protocols)
