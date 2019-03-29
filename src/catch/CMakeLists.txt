cmake_minimum_required(VERSION 2.8)
project(catch_builder CXX)
include(ExternalProject)

ExternalProject_Add(
    catch
    PREFIX ${CMAKE_BINARY_DIR}/catch
    URL https://github.com/catchorg/Catch2/archive/v2.7.0.tar.gz
    URL_HASH SHA1=6df37d5b64a71b840a6a9d8c79c3705aa8a3f56e
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/single_include CACHE INTERNAL "Path to include folder for Catch")
