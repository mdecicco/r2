project(entity_test)

file(GLOB_RECURSE 10_entity_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(10_entity_test ${10_entity_test_src})
 
SOURCE_GROUP("" FILES ${10_entity_test_src})

target_include_directories(10_entity_test PUBLIC ../../engine)
target_link_libraries(10_entity_test r2)
