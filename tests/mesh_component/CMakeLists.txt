project(mesh_component_test)

file(GLOB_RECURSE 11_mesh_component_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(11_mesh_component_test ${11_mesh_component_test_src})
 
SOURCE_GROUP("" FILES ${11_mesh_component_test_src})

target_include_directories(11_mesh_component_test PUBLIC ../../engine)
target_link_libraries(11_mesh_component_test r2)
