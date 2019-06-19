cmake_minimum_required (VERSION 3.14.5)
project (workcrew)

set (workcrew_VERSION_MAJOR 1)
set (workcrew_VERSION_MINOR 0)
 
# add the executable
add_executable(workcrew workcrew.cc)
target_link_libraries(workcrew pthread)
