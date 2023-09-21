cmake_minimum_required(VERSION 3.25)
project(STC_Task)

set(CMAKE_CXX_STANDARD 20)

Include(FetchContent)

FetchContent_Declare(
        GoogleTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(GoogleTest)

add_subdirectory(src)
add_subdirectory(test)