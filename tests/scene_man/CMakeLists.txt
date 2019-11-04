project(scene_man_test)

add_executable(2_scene_test main.cpp)

target_include_directories(2_scene_test PUBLIC ../../engine)
target_link_libraries(2_scene_test r2)
