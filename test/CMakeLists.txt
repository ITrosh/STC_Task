set(TEST_TARGET STC_TaskTest)

set(TEST_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/STC_TaskTest.cpp
)

add_executable(${TEST_TARGET} ${TEST_SOURCES})

target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest IOStreamByteConverter)