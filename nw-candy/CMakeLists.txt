#policies first
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0076 NEW)
endif()


###################
# delcare library #
###################

add_library(nw-candy STATIC)

target_sources(nw-candy PRIVATE
    src/uPnPForwarder.cpp
    src/IGDv1Forwarder.cpp
    src/IGDv2Forwarder.cpp
    src/uPnPHandler.cpp
    src/ConnectivityManager.cpp
)

target_include_directories(nw-candy
    PRIVATE include/nw-candy
    INTERFACE include
)

# link
target_link_libraries(nw-candy PRIVATE ole32)

####################
# Deps : miniupnpc #
####################

#find miniupnpc
find_package(miniupnpc REQUIRED)

# on MacOS brew, points directly to miniupnpc folder, we do not want that
if(APPLE)
    get_filename_component(MINIUPNP_INCLUDE_DIR ${MINIUPNP_INCLUDE_DIR} DIRECTORY)
endif()

#define
add_library(miniupnpc INTERFACE)
target_link_libraries(miniupnpc INTERFACE ${MINIUPNP_LIBRARY})
target_include_directories(miniupnpc INTERFACE ${MINIUPNP_INCLUDE_DIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(miniupnpc INTERFACE ws2_32)
endif()

#link
target_link_libraries(nw-candy PUBLIC miniupnpc)

#################
# Deps : spdlog #
#################

#find
if(NOT TARGET spdlog::spdlog)
    find_package(spdlog REQUIRED)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW)
        # https://github.com/msys2/MINGW-packages/issues/6855
        target_link_options(spdlog::spdlog INTERFACE "-Wl,/force:multiple")
    endif()
endif()

#link
target_link_libraries(nw-candy PUBLIC spdlog::spdlog)