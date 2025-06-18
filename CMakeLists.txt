cmake_minimum_required(VERSION 3.14)
project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Require MIDASSYS environment variable
if(NOT DEFINED ENV{MIDASSYS})
  message(FATAL_ERROR
    "Environment variable MIDASSYS not set.\n"
    "Please source your MIDAS environment or run the detect_environment.sh script:\n"
    "  ./scripts/environment/detect_environment.sh\n"
    "  source ./scripts/environment/.env"
  )
endif()

# MIDAS include dirs and lib dir
set(MIDASSYS_INCLUDE_DIRS
  $ENV{MIDASSYS}/include
  $ENV{MIDASSYS}/midasio
)
set(MIDASSYS_LIB_DIR $ENV{MIDASSYS}/lib)

# Project include directory (absolute path)
get_filename_component(PROJECT_INCLUDE_DIR_ABS ${CMAKE_CURRENT_SOURCE_DIR}/include ABSOLUTE)

# Collect source files recursively (absolute paths guaranteed by GLOB_RECURSE)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Collect headers for ROOT dictionary, excluding LinkDef.h explicitly
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS
  ${PROJECT_INCLUDE_DIR_ABS}/stages/*.h
)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to source ROOT environment (e.g. 'source thisroot.sh').")
endif()

include(${ROOT_USE_FILE})

# FetchContent for spdlog and nlohmann_json
include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# ROOT dictionary generation — use absolute paths everywhere
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${PROJECT_INCLUDE_DIR_ABS}/stages/LinkDef.h
  OPTIONS
    -I${PROJECT_INCLUDE_DIR_ABS}
    -I$ENV{MIDASSYS}/include
    -I$ENV{MIDASSYS}/midasio
    -Wno-inconsistent-missing-override
)

# Create shared library with source files and generated dictionary (use absolute path to generated source)
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__Stages.cxx
)

# Include directories (PUBLIC so consumers get includes transitively)
target_include_directories(analysis_pipeline_stages PUBLIC
  ${PROJECT_INCLUDE_DIR_ABS}
  ${MIDASSYS_INCLUDE_DIRS}
)

# Link libraries (explicit full paths for MIDAS libs, targets for others)
target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  ${MIDASSYS_LIB_DIR}/libmfe.a
  ${MIDASSYS_LIB_DIR}/libmidas.a
)

# Install rules
install(TARGETS analysis_pipeline_stages
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
