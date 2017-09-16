project(event_test)

add_executable(test4 main.cpp)

target_include_directories(test4 PUBLIC ../../engine)
target_link_libraries(test4 r2)
