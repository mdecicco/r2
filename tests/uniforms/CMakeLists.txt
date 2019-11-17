project(uniforms_test)

file(GLOB_RECURSE 8_uniforms_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(8_uniforms_test ${8_uniforms_test_src})
 
SOURCE_GROUP("" FILES ${8_uniforms_test_src})

target_include_directories(8_uniforms_test PUBLIC ../../engine)
target_link_libraries(8_uniforms_test r2)
