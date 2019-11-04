project(mesh_test)

add_executable(3_mesh_test main.cpp)

target_include_directories(3_mesh_test PUBLIC ../../engine)
target_link_libraries(3_mesh_test r2)
