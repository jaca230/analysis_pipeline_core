cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineStagesCore VERSION 0.1 LANGUAGES CXX)

# General settings
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Standard CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Dependencies
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
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

target_include_directories(analysis_pipeline_stages_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages_core>
)

target_link_libraries(analysis_pipeline_stages_core PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Install rules
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS analysis_pipeline_stages_core
    EXPORT AnalysisPipelineStagesCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
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
