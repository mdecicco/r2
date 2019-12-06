project(mesh_test)

file(GLOB_RECURSE 5_mesh_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(5_mesh_test ${5_mesh_test_src})
 
SOURCE_GROUP("" FILES ${5_mesh_test_src})

target_include_directories(5_mesh_test PUBLIC ../../engine)
target_link_libraries(5_mesh_test r2)
set_property(TARGET 5_mesh_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")
