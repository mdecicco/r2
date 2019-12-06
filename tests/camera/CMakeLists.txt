project(camera_test)

file(GLOB_RECURSE 9_camera_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(9_camera_test ${9_camera_test_src})
 
SOURCE_GROUP("" FILES ${9_camera_test_src})

target_include_directories(9_camera_test PUBLIC ../../engine)
target_link_libraries(9_camera_test r2)

set_property(TARGET 9_camera_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")
