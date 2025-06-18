cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include(FetchContent)

# Disable installing spdlog/nlohmann_json since we'll handle their export manually
set(spdlog_INSTALL OFF CACHE BOOL "" FORCE)
set(nlohmann_json_INSTALL OFF CACHE BOOL "" FORCE)

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

# ROOT setup
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
if(NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT not found. Make sure to 'source thisroot.sh'")
endif()
include(${ROOT_USE_FILE})

# Source/header files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# ROOT dictionary
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/LinkDef.h
  OPTIONS -I${CMAKE_CURRENT_SOURCE_DIR}/include -Wno-inconsistent-missing-override
)

# Build the library
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__Stages.cxx
)

set_target_properties(analysis_pipeline_stages PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

# Include paths
target_include_directories(analysis_pipeline_stages PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/analysis_pipeline_stages>
)

# Link dependencies
target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
)

# Install rules (skip in superproject context)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  include(GNUInstallDirs)

  # Install your library target
  install(TARGETS analysis_pipeline_stages
    EXPORT AnalysisPipelineStagesTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  # Also install header-only dependencies targets to export set
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

  # Install headers
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  # Export targets file
  install(EXPORT AnalysisPipelineStagesTargets
    FILE AnalysisPipelineStagesTargets.cmake
    NAMESPACE AnalysisPipelineStages::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
    EXPORT_LINK_INTERFACE_LIBRARIES
  )

  # Package config version file
  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )
endif()
