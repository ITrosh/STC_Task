add_library(IOStreamByteConverter STATIC
    ${CMAKE_CURRENT_LIST_DIR}/IOStreamByteConverter.h
)

target_include_directories(IOStreamByteConverter PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(IOStreamByteConverter PUBLIC Logger)