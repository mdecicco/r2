project(event_test)

file(GLOB_RECURSE 2_event_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(2_event_test ${2_event_test_src})
 
SOURCE_GROUP("" FILES ${2_event_test_src})

target_include_directories(2_event_test PUBLIC ../../engine)
target_link_libraries(2_event_test r2)