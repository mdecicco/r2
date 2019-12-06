project(scene_man_test)

file(GLOB_RECURSE 4_scene_man_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(4_scene_man_test ${4_scene_man_test_src})
 
SOURCE_GROUP("" FILES ${4_scene_man_test_src})

target_include_directories(4_scene_man_test PUBLIC ../../engine)
target_link_libraries(4_scene_man_test r2)
set_property(TARGET 4_scene_man_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")
