cmake_minimum_required(VERSION 3.14)
project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Include directories
file(GLOB_RECURSE ALL_INCLUDE_PATHS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/include/*)
foreach(path ${ALL_INCLUDE_PATHS})
    if(IS_DIRECTORY ${path})
        include_directories(${path})
    endif()
endforeach()

# Source and header files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Glob all stage headers except LinkDef.h for dictionary generation
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS include/stages/*.h)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# ROOT
find_package(ROOT REQUIRED COMPONENTS Tree Core Hist RIO)
if(NOT ROOT_FOUND)
    message(FATAL_ERROR "ROOT not found. Make sure to source ROOT (e.g. 'source thisroot.sh').")
endif()
include(${ROOT_USE_FILE})

# spdlog
include(FetchContent)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Dictionary generation using filtered headers (exclude LinkDef.h) and LinkDef.h last
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_SOURCE_DIR}/include/stages/LinkDef.h
)

# Shared library
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  G__Stages.cxx
)

target_include_directories(analysis_pipeline_stages PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Install rule
install(TARGETS analysis_pipeline_stages
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
