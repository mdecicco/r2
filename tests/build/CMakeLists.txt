project(build_test)

file(GLOB_RECURSE 0_build_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(0_build_test ${0_build_test_src})
 
SOURCE_GROUP("" FILES ${0_build_test_src})

target_include_directories(0_build_test PUBLIC ../../engine)
target_link_libraries(0_build_test r2)
set_property(TARGET 0_build_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")