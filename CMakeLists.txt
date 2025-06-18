cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

# 1. Make PIC the default globally, so all libraries (and executables) get compiled position independent
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 2. Enable generation of compile_commands.json for IDEs/tools (Ninja, VSCode, clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Disable installing spdlog/nlohmann_json; this project handles export/install manually
set(spdlog_INSTALL OFF CACHE BOOL "" FORCE)
set(nlohmann_json_INSTALL OFF CACHE BOOL "" FORCE)

# Fetch header-only dependencies
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

# Find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to 'source thisroot.sh'")
endif()
include(${ROOT_USE_FILE})

# Gather sources and headers
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# Generate ROOT dictionary
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/LinkDef.h
  OPTIONS -I${CMAKE_CURRENT_SOURCE_DIR}/include -Wno-inconsistent-missing-override
)

# Add EXPORT to library target for easier export handling
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__Stages.cxx
)

# Set version properties for the shared library
set_target_properties(analysis_pipeline_stages PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

# Use target_compile_features instead of setting CMAKE_CXX_STANDARD globally
target_compile_features(analysis_pipeline_stages PUBLIC cxx_std_17)

target_include_directories(analysis_pipeline_stages PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages>
)

# Link dependencies publicly (transitive)
target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Alias target for submodule usage convenience (optional)
add_library(AnalysisPipelineStages::analysis_pipeline_stages ALIAS analysis_pipeline_stages)

# Export the package for find_package support
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  # Install library target
  install(TARGETS analysis_pipeline_stages
    EXPORT AnalysisPipelineStagesTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  # Install headers
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  # Export header-only dependencies into the export set for transitive consumption
  if(TARGET spdlog::spdlog)
    install(TARGETS spdlog
      EXPORT AnalysisPipelineStagesTargets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spdlog
    )
  endif()

  if(TARGET nlohmann_json::nlohmann_json)
    install(TARGETS nlohmann_json
      EXPORT AnalysisPipelineStagesTargets
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nlohmann_json
    )
  endif()

  # Export targets for find_package and usage
  install(EXPORT AnalysisPipelineStagesTargets
    FILE AnalysisPipelineStagesTargets.cmake
    NAMESPACE AnalysisPipelineStages::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
    EXPORT_LINK_INTERFACE_LIBRARIES
  )

  # Generate and install the ConfigVersion.cmake file
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  # Create and install Config.cmake for find_package support
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  # Export the package to enable find_package
  export(PACKAGE AnalysisPipelineStages)
endif()
