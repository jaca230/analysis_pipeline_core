cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineStagesCore VERSION 0.1 LANGUAGES CXX)

# General settings
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Suppress false-positive GCC warnings when top-level
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-stringop-overflow)
  endif()
endif()

# Standard CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --------------------- CPM Setup ---------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

# Use externally-defined versions if set, else fallback
set(CPM_SPDLOG_VERSION "1.13.0" CACHE STRING "spdlog version")
set(CPM_NLOHMANN_JSON_VERSION "3.11.3" CACHE STRING "nlohmann_json version")

# --------------------- spdlog ---------------------
CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION ${CPM_SPDLOG_VERSION}
  OPTIONS
    "SPDLOG_BUILD_EXAMPLES OFF"
    "SPDLOG_BUILD_TESTS OFF"
    "SPDLOG_FMT_EXTERNAL OFF"
  DOWNLOAD_ONLY YES
)

if(spdlog_ADDED)
  add_library(spdlog_header_only INTERFACE)
  target_include_directories(spdlog_header_only INTERFACE
    $<BUILD_INTERFACE:${spdlog_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  add_library(spdlog::spdlog ALIAS spdlog_header_only)
endif()

# --------------------- nlohmann_json ---------------------
CPMAddPackage(
  NAME nlohmann_json
  GITHUB_REPOSITORY nlohmann/json
  VERSION ${CPM_NLOHMANN_JSON_VERSION}
  DOWNLOAD_ONLY YES
)

if(nlohmann_json_ADDED)
  add_library(nlohmann_json_header_only INTERFACE)
  target_include_directories(nlohmann_json_header_only INTERFACE
    $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json_header_only)
endif()

# Find ROOT (still required for dictionary generation and linking)
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})

# Source and header files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h
)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# ROOT dictionary generation
ROOT_GENERATE_DICTIONARY(G__StagesCore
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/LinkDef.h
  OPTIONS
    -I${CMAKE_CURRENT_SOURCE_DIR}/include
    -Wno-inconsistent-missing-override
)

# Build shared library
add_library(analysis_pipeline_stages_core SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__StagesCore.cxx
)

add_library(AnalysisPipelineStagesCore::analysis_pipeline_stages_core ALIAS analysis_pipeline_stages_core)

set_target_properties(analysis_pipeline_stages_core PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_compile_features(analysis_pipeline_stages_core PUBLIC cxx_std_17)

# Include directories: your headers + CPM downloaded header-only libs
target_include_directories(analysis_pipeline_stages_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages_core>
)

# Linking
target_link_libraries(analysis_pipeline_stages_core PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  spdlog_header_only
  nlohmann_json_header_only
)

# Install rules
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS analysis_pipeline_stages_core
    EXPORT AnalysisPipelineStagesCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages_core
  )

  install(TARGETS spdlog_header_only nlohmann_json_header_only
    EXPORT AnalysisPipelineStagesCoreTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages_core
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages_core
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libStagesCore.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libStagesCore_rdict.pcm
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  install(EXPORT AnalysisPipelineStagesCoreTargets
    FILE AnalysisPipelineStagesCoreTargets.cmake
    NAMESPACE AnalysisPipelineStagesCore::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStagesCore
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStagesCore
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStagesCore
  )

  export(PACKAGE AnalysisPipelineStagesCore)

endif()
