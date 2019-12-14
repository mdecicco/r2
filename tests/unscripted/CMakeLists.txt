project(unscripted_test)

file(GLOB_RECURSE 12_unscripted_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(12_unscripted_test ${12_unscripted_test_src})
 
SOURCE_GROUP("" FILES ${12_unscripted_test_src})

target_include_directories(12_unscripted_test PUBLIC ../../engine)
target_link_libraries(12_unscripted_test r2)
