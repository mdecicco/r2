project(mesh_test)

add_executable(test3 main.cpp)

target_include_directories(test3 PUBLIC ../../engine)
target_link_libraries(test3 r2)
