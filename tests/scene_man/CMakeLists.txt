project(scene_man_test)

add_executable(test1 main.cpp)

target_include_directories(test1 PUBLIC ../../engine)
target_link_libraries(test1 r2)
