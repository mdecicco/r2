@echo off

set test_number=
for /d %%a in (.\*) do set /a test_number+=1

set /p test_name="Enter test name: "

set project_name=%test_name%_test
set executable_name=%test_number%_%test_name%_test

echo add_subdirectory(%test_name%)>> CMakeLists.txt

mkdir %test_name%
copy default_main.cpp "./%test_name%/main.cpp" >nul
cd %test_name%

echo project(%test_name%_test)>> CMakeLists.txt
echo.>> CMakeLists.txt
echo file(GLOB_RECURSE %executable_name%_src "*.h" "*.cpp")>> CMakeLists.txt
echo set_property(GLOBAL PROPERTY USE_FOLDERS ON)>> CMakeLists.txt
echo add_executable(%executable_name% ${%executable_name%_src})>> CMakeLists.txt
echo. >> CMakeLists.txt
echo SOURCE_GROUP("" FILES ${%executable_name%_src})>> CMakeLists.txt
echo.>> CMakeLists.txt
echo target_include_directories(%executable_name% PUBLIC ../../engine)>> CMakeLists.txt
echo target_link_libraries(%executable_name% r2)>> CMakeLists.txt

pause