project(build_test)

add_executable(test0 main.cpp)

target_include_directories(test0 PUBLIC ../../engine)
target_link_libraries(test0 r2)
