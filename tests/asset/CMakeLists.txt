project(asset_test)

file(GLOB_RECURSE 1_asset_test_src "*.h" "*.cpp")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_executable(1_asset_test ${1_asset_test_src})
 
SOURCE_GROUP("" FILES ${1_asset_test_src})

target_include_directories(1_asset_test PUBLIC ../../engine)
target_link_libraries(1_asset_test r2)
set_property(TARGET 1_asset_test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")