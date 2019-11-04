project(build_test)

add_executable(0_build_test main.cpp)

target_include_directories(0_build_test PUBLIC ../../engine)
target_link_libraries(0_build_test r2)
