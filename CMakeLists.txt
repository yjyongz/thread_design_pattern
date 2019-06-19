cmake_minimum_required (VERSION 3.14.5)
project (thread_design)

set (thread_design_VERSION_MAJOR 1)
set (thread_design_VERSION_MINOR 0)

include(ExternalProject)

ExternalProject_Add ( check-thread_pool
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/thread_pool"
  PREFIX "thread_pool" 
  INSTALL_COMMAND "" ) 

ExternalProject_Add ( check-workcrew
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/workcrew"
  PREFIX "workcrew" 
  INSTALL_COMMAND "" ) 
