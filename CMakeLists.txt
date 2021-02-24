cmake_minimum_required(VERSION 3.10.2) #stick to 18.04 Ubuntu LTS version

project(NetworkCandy
    VERSION 0.1.2
    DESCRIPTION "Network system notifications and uPnP shenanigans"
    HOMEPAGE_URL "https://github.com/Amphaal/NetworkCandy"
    LANGUAGES CXX
)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(nw-candy)

#########
# Tests #
#########

#dont build tests if included as submodule
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(tests)
endif()
