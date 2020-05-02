project(libr2)

file(GLOB_RECURSE r2_src
    "r2/*.h"
    "r2/*.cpp"
    "r2/managers/*.h"
    "r2/managers/*.cpp"
    "r2/managers/drivers/gl/*.h"
    "r2/managers/drivers/gl/*.cpp"
    "r2/systems/*.h"
    "r2/systems/*.cpp"
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

target_include_directories(r2 PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}\\include"
	"${CMAKE_CURRENT_SOURCE_DIR}\\include\\OIS"
	"C:\\Program Files (x86)\\OpenAL 1.1 SDK\\include"
)

target_link_directories(r2 PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	"C:\\Program Files (x86)\\OpenAL 1.1 SDK\\libs\\Win64"
)

SET(debug_libs
	debug/v8pp
	debug/v8
	debug/v8_libbase
	debug/v8_libplatform
	debug/glfw3
	debug/OIS_d
	debug/Bullet3Collision_Debug
	debug/Bullet3Common_Debug
	debug/Bullet3Dynamics_Debug
	debug/Bullet3Geometry_Debug
	debug/BulletCollision_Debug
	debug/BulletDynamics_Debug
	debug/LinearMath_Debug
	debug/marl
	OpenAL32
	opengl32
)
SET(release_libs
	release/v8pp
	release/v8
	release/v8_libbase
	release/v8_libplatform
	release/glfw3
	release/OIS
	release/Bullet3Collision
	release/Bullet3Common
	release/Bullet3Dynamics
	release/Bullet3Geometry
	release/BulletCollision
	release/BulletDynamics
	release/LinearMath
	release/marl
	OpenAL32
	opengl32
)

target_link_libraries(r2 ${debug_libs})
#target_link_libraries(r2 ${release_libs})

add_definitions(-D_CRT_NO_VA_START_VALIDATION)