cmake_minimum_required(VERSION 3.14)
project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for built libraries (relative to top-level source directory)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Check MIDASSYS environment variable
if(NOT DEFINED ENV{MIDASSYS})
  message(FATAL_ERROR
    "Environment variable MIDASSYS not set.\n"
    "Please source your MIDAS environment or run the detect_environment.sh script:\n"
    "  ./scripts/environment/detect_environment.sh\n"
    "  source ./scripts/environment/.env"
  )
endif()

# MIDAS include and lib directories
set(MIDASSYS_INCLUDE_DIRS
  $ENV{MIDASSYS}/include
  $ENV{MIDASSYS}/midasio
)
set(MIDASSYS_LIB_DIR $ENV{MIDASSYS}/lib)

# Include root directory (top-level include folder)
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Headers for ROOT dictionary generation (exclude LinkDef.h)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS
  ${PROJECT_INCLUDE_DIR}/stages/*.h
)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Tree Core Hist RIO)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to source ROOT (e.g. 'source thisroot.sh').")
endif()
include(${ROOT_USE_FILE})

# FetchContent for spdlog
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# FetchContent for nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Generate ROOT dictionary with proper include dirs for your project and MIDAS
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${PROJECT_INCLUDE_DIR}/stages/LinkDef.h
  OPTIONS 
    -I${PROJECT_INCLUDE_DIR} 
    -I$ENV{MIDASSYS}/include 
    -I$ENV{MIDASSYS}/midasio 
    -Wno-inconsistent-missing-override
)

# Create shared library for analysis pipeline stages
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  G__Stages.cxx
)

# Include directories for the target
target_include_directories(analysis_pipeline_stages PUBLIC
  ${PROJECT_INCLUDE_DIR}      # Add your top-level include
  ${MIDASSYS_INCLUDE_DIRS}
)

# Link directories for MIDASSYS libs
link_directories(${MIDASSYS_LIB_DIR})

# Link libraries
target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Installation rules
install(TARGETS analysis_pipeline_stages
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
