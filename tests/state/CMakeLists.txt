project(state_test)

add_executable(5_state_test main.cpp)

target_include_directories(5_state_test PUBLIC ../../engine)
target_link_libraries(5_state_test r2)
