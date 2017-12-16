cmake_minimum_required(VERSION 2.8)
project(catch_builder CXX)
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    catch
    PREFIX ${CMAKE_BINARY_DIR}/catch
    URL https://github.com/catchorg/Catch2/releases/download/v2.0.1/catch.hpp
    DOWNLOAD_NO_EXTRACT 1
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
   )

# Expose required variable (CATCH_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/../ CACHE INTERNAL "Path to include folder for Catch")
