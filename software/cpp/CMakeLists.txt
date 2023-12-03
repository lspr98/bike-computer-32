cmake_minimum_required (VERSION 3.0)

# Set project name
project(osm2simpletile)

set(CMAKE_CXX_FLAGS "-Wall")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -fopenmp")

# Add cmake folder to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Require openmp
find_package(OpenMP REQUIRED)

# Require libosmium
find_package(Osmium REQUIRED)
if(NOT OSMIUM_FOUND)
    message(WARNING "Libosmium not found!\n")
endif()
find_package(Osmium REQUIRED COMPONENTS io pbf gdal xml)

include_directories(include)
include_directories(${OSMIUM_INCLUDE_DIRS})

# Define entry point and executable

add_executable(osm2simpletile main.cpp)

target_link_libraries(osm2simpletile ${OSMIUM_LIBRARIES})