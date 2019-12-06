project(timer_test)

file(GLOB_RECURSE 7_timer_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(7_timer_test ${7_timer_test_src})
 
SOURCE_GROUP("" FILES ${7_timer_test_src})

target_include_directories(7_timer_test PUBLIC ../../engine)
target_link_libraries(7_timer_test r2)
set_property(TARGET 7_timer_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")