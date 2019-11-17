project(libr2)

file(GLOB_RECURSE r2_src
    "r2/*.h"
    "r2/*.cpp"
    "r2/managers/*.h"
    "r2/managers/*.cpp"
    "r2/managers/drivers/gl/*.h"
    "r2/managers/drivers/gl/*.cpp"
    "r2/utilities/*.h"
    "r2/utilities/*.hpp"
    "r2/utilities/*.cpp"
    "r2/utilities/*.c"
    "r2/utilities/imgui/*.h"
    "r2/utilities/imgui/*.cpp"
    "r2/bindings/*.h"
    "r2/bindings/*.cpp"
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_library(r2 ${r2_src})
MACRO(SOURCE_GROUP_BY_FOLDER target)
  SET(SOURCE_GROUP_DELIMITER "/")
  SET(last_dir "")
  SET(files "")
  FOREACH(file ${${target}_src})
    file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}/${target}" ${file})
    GET_FILENAME_COMPONENT(dir "${relative_file}" PATH)
    IF (NOT "${dir}" STREQUAL "${last_dir}")
      IF (files)
        SOURCE_GROUP("${last_dir}" FILES ${files})
      ENDIF (files)
      SET(files "")
    ENDIF (NOT "${dir}" STREQUAL "${last_dir}")
    SET(files ${files} ${file})
    SET(last_dir "${dir}")
  ENDFOREACH(file)
  IF (files)
    SOURCE_GROUP("${last_dir}" FILES ${files})
  ENDIF (files)
ENDMACRO(SOURCE_GROUP_BY_FOLDER)
SOURCE_GROUP_BY_FOLDER(r2)

target_include_directories(r2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}\\include")
target_link_directories(r2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(r2 v8pp v8 v8_libbase v8_libplatform glfw3 opengl32)

add_definitions(-D_CRT_NO_VA_START_VALIDATION)