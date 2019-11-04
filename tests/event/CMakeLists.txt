project(event_test)

add_executable(4_event_test main.cpp)

target_include_directories(4_event_test PUBLIC ../../engine)
target_link_libraries(4_event_test r2)
