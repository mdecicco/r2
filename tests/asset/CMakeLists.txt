project(asset_test)

add_executable(test2 main.cpp)

target_include_directories(test2 PUBLIC ../../engine)
target_link_libraries(test2 r2)
