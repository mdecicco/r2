project(state_test)

file(GLOB_RECURSE 3_state_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(3_state_test ${3_state_test_src})
 
SOURCE_GROUP("" FILES ${3_state_test_src})

target_include_directories(3_state_test PUBLIC ../../engine)
target_link_libraries(3_state_test r2)
set_property(TARGET 3_state_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")