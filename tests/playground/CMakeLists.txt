project(playground_test)

file(GLOB_RECURSE 13_playground_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(13_playground_test ${13_playground_test_src})
 
SOURCE_GROUP("" FILES ${13_playground_test_src})

target_include_directories(13_playground_test PUBLIC ../../engine)
target_link_libraries(13_playground_test r2)
