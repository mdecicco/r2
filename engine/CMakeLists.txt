project(libr2)

file(GLOB r2_src
    "r2/*.h"
    "r2/*.cpp"
    "r2/managers/*.h"
    "r2/managers/*.cpp"
    "r2/utilities/*.h"
    "r2/utilities/*.cpp"
)

add_library(r2 ${r2_src})

target_include_directories(r2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
