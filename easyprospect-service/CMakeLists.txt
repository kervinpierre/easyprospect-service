cmake_minimum_required(VERSION 3.5)

project(easyprospect-service)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}/${CMAKE_SYSTEM_NAME}/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}/${CMAKE_SYSTEM_NAME}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}/${CMAKE_SYSTEM_NAME}/debug)

#if (UNIX)
#    link_directories(libs/)
#    include_directories(include/v8/)
#endif (UNIX)
#
#if (WIN32)
#    link_directories(packages/v8-v141-x64.7.4.288.26/lib/)
#    include_directories(packages/v8-v141-x64.7.4.288.26/include/)
#    include_directories(include/)
#endif (WIN32)

include_directories(include/)

#set(Boost_USE_STATIC_LIBS        ON) # only find static libs
#set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost REQUIRED COMPONENTS system thread regex)
message("this is ${Boost_INCLUDE_DIRS}")

find_package(OpenSSL REQUIRED)

add_library(EPConfigLib STATIC
        src/easyprospect-config/easyprospect-config.cpp
        include/easyprospect-config/easyprospect-config.h
        )

add_library(EPWebLib STATIC
        src/easyprospect-web/easyprospect-web.cpp
        include/easyprospect-web/easyprospect-web.h
        )

target_include_directories(EPConfigLib PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(EPWebLib PRIVATE ${Boost_INCLUDE_DIRS})

#target_include_directories(easyprospect-service PUBLIC ${CMAKE_SOURCE_DIR})

#target_compile_definitions(easyprospect-service PUBLIC EASY_PROJECT_LIBRARY)

add_executable( EPSrv
        src/epsrv/epsrv.cpp )

target_include_directories(EPSrv PRIVATE ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
#target_link_libraries(EPConfigLib PRIVATE ${Boost_LIBRARIES})

target_link_libraries( EPSrv
        EPConfigLib
        EPWebLib
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES} )

#if (UNIX)
#    target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g --coverage)
#endif (UNIX)
#
#if (UNIX)
#    target_link_libraries(${PROJECT_NAME} PUBLIC icuuc icui18n v8_libplatform v8_libbase v8)
#
#    if (NOT CMAKE_VERSION VERSION_LESS 3.13)
#        target_link_options(${PROJECT_NAME} PUBLIC -ftest-coverage --coverage)
#    else ()
#        target_link_libraries(${PROJECT_NAME} PUBLIC -ftest-coverage --coverage)
#    endif ()
#
#endif (UNIX)
#
#if (WIN32)
#    target_link_libraries(${PROJECT_NAME} v8_libbase.dll.lib v8_libplatform.dll.lib v8.dll.lib)
#endif (WIN32)
#
#if (UNIX)
#    add_custom_command(TARGET easyprospect-service POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/build/ $<TARGET_FILE:easyprospect-service> ${CMAKE_SOURCE_DIR}/../bin/)
#    add_custom_command(TARGET easyprospect-service POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/build/ $<TARGET_FILE:easyprospect-service> ${CMAKE_SOURCE_DIR}/build/tests/)
#endif (UNIX)
#
#enable_testing()
#
#add_subdirectory(tests)
#
#if (DOCS)
#    add_subdirectory("docs")
#endif ()