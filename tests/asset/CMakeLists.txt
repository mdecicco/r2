project(asset_test)

add_executable(1_asset_test main.cpp)

target_include_directories(1_asset_test PUBLIC ../../engine)
target_link_libraries(1_asset_test r2)
