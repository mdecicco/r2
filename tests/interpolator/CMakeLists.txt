project(interpolator_test)

file(GLOB_RECURSE 6_interpolator_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(6_interpolator_test ${6_interpolator_test_src})
 
SOURCE_GROUP("" FILES ${6_interpolator_test_src})

target_include_directories(6_interpolator_test PUBLIC ../../engine)
target_link_libraries(6_interpolator_test r2)
