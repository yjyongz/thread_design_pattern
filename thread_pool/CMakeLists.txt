cmake_minimum_required (VERSION 3.14.5)
project (Thread_pool)

set (Thread_pool_VERSION_MAJOR 1)
set (Thread_pool_VERSION_MINOR 0)
 
# add the executable
add_executable(Thread_pool thread_pool.cc)
target_link_libraries(Thread_pool pthread)
