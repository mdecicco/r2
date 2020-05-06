project(scripted_system_test)

file(GLOB_RECURSE 15_scripted_system_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(15_scripted_system_test ${15_scripted_system_test_src})
 
SOURCE_GROUP("" FILES ${15_scripted_system_test_src})

target_include_directories(15_scripted_system_test PUBLIC ../../engine)
target_link_libraries(15_scripted_system_test r2)
