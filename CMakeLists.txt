cmake_minimum_required(VERSION 3.14)
project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for built libraries (relative to top-level source directory)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Include directories - recursively add all include subfolders under this submodule
file(GLOB_RECURSE ALL_INCLUDE_DIRS LIST_DIRECTORIES true
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*
)
set(UNIQUE_INCLUDE_DIRS "")
foreach(path IN LISTS ALL_INCLUDE_DIRS)
  if(IS_DIRECTORY ${path})
    list(APPEND UNIQUE_INCLUDE_DIRS ${path})
  endif()
endforeach()

list(REMOVE_DUPLICATES UNIQUE_INCLUDE_DIRS)

# Source files (recursively under src/)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Header files for dictionary generation (exclude LinkDef.h)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h
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

# Generate ROOT dictionary
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/LinkDef.h
)

# Create shared library for analysis pipeline stages
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  G__Stages.cxx
)

# Include directories for the target
target_include_directories(analysis_pipeline_stages PUBLIC
  ${UNIQUE_INCLUDE_DIRS}
)

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
