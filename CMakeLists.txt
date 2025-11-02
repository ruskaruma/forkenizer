cmake_minimum_required(VERSION 3.20)
project(forkenizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/tokenizer/Tokenizer.cpp
    src/tokenizer/PreTokenizer.cpp
    src/io/ModelIO.cpp
)

add_library(forkenizer STATIC ${SOURCES})

target_include_directories(forkenizer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(forkenizer PRIVATE -Wall -Wextra -Werror)

set(CLI_SOURCES
    src/cli/main.cpp
    src/trainer/Trainer.cpp
)

add_executable(forkenizer-cli ${CLI_SOURCES})
target_link_libraries(forkenizer-cli forkenizer)

if(BUILD_TESTS)
    enable_testing()
    add_executable(forkenizer-tests
        tests/unit/test_encode_decode.cpp
        tests/unit/test_numbers_math.cpp
        tests/unit/test_pretokenizer.cpp
    )
    target_link_libraries(forkenizer-tests forkenizer)
    target_include_directories(forkenizer-tests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
    add_test(NAME forkenizer-tests COMMAND forkenizer-tests)
endif()

