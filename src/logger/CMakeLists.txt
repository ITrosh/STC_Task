add_library(Logger STATIC
    ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
)

target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_LIST_DIR})