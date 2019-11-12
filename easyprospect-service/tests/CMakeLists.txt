cmake_minimum_required(VERSION 3.5)

project(easyprospect-v8-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

link_directories(${PROJECT_NAME} ../bin/)

if (WIN32)
link_directories(../packages/v8-v141-x64.7.4.288.26/lib/)
link_directories(../build)
endif (WIN32)

add_executable(${PROJECT_NAME}
    main.cpp)

include_directories(../include/)

if (UNIX)
target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g --coverage)

target_link_libraries(${PROJECT_NAME} PUBLIC v8_libplatform v8_libbase v8 easyprospect-v8)

if (NOT CMAKE_VERSION VERSION_LESS 3.13)
target_link_options(${PROJECT_NAME} PUBLIC -ftest-coverage --coverage)
else ()
target_link_libraries(${PROJECT_NAME} PUBLIC -ftest-coverage --coverage)
endif ()

endif (UNIX)

if (WIN32)
target_link_libraries(${PROJECT_NAME} easyprospect-v8)
include_directories(../packages/v8-v141-x64.7.4.288.26/include/)
include_directories(include/)
target_link_libraries(${PROJECT_NAME} v8_libbase.dll.lib v8_libplatform.dll.lib v8.dll.lib)
endif (WIN32)

if (UNIX)
add_custom_command(TARGET easyprospect-v8-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/build/ $<TARGET_FILE:easyprospect-v8-test> ${CMAKE_SOURCE_DIR}/../bin/)
add_custom_command(TARGET easyprospect-v8-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../bin/natives_blob.bin ${CMAKE_SOURCE_DIR}/build/tests/)
add_custom_command(TARGET easyprospect-v8-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/../bin/snapshot_blob.bin ${CMAKE_SOURCE_DIR}/build/tests/)
endif (UNIX)

add_test(NAME test COMMAND easyprospect-v8-test)
