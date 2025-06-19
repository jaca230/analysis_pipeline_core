cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Enforce modern C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies (require consumer to provide them)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
include(${ROOT_USE_FILE})

# Gather sources/headers
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# ROOT dictionary
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/LinkDef.h
  OPTIONS -I${CMAKE_CURRENT_SOURCE_DIR}/include -Wno-inconsistent-missing-override
)

# Shared library
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__Stages.cxx
)

set_target_properties(analysis_pipeline_stages PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_compile_features(analysis_pipeline_stages PUBLIC cxx_std_17)

target_include_directories(analysis_pipeline_stages PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages>
)

target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

add_library(AnalysisPipelineStages::analysis_pipeline_stages ALIAS analysis_pipeline_stages)

# Install logic
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS analysis_pipeline_stages
    EXPORT AnalysisPipelineStagesTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libStages.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libStages_rdict.pcm
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  install(EXPORT AnalysisPipelineStagesTargets
    FILE AnalysisPipelineStagesTargets.cmake
    NAMESPACE AnalysisPipelineStages::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
    EXPORT_LINK_INTERFACE_LIBRARIES
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  export(PACKAGE AnalysisPipelineStages)
endif()
