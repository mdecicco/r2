project(physics_test)

file(GLOB_RECURSE 14_physics_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(14_physics_test ${14_physics_test_src})
 
SOURCE_GROUP("" FILES ${14_physics_test_src})

target_include_directories(14_physics_test PUBLIC ../../engine)
target_link_libraries(14_physics_test r2)
