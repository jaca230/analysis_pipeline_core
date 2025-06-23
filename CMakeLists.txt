cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineStages VERSION 0.1 LANGUAGES CXX)

# General settings
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Standard CMake modules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Require MIDASSYS environment
if(NOT DEFINED ENV{MIDASSYS})
  message(FATAL_ERROR
    "Environment variable MIDASSYS not set.\n"
    "Please source your MIDAS environment or run the detect_environment.sh script."
  )
endif()

set(MIDASSYS_INCLUDE_DIRS
  $ENV{MIDASSYS}/include
  $ENV{MIDASSYS}/midasio
)
set(MIDASSYS_LIB_DIR $ENV{MIDASSYS}/lib)

# Find dependencies
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
find_package(unpacker REQUIRED)

include(${ROOT_USE_FILE})

# Gather source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Gather stage headers
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/*.h
)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

# Get all unpacker targets you link
set(UNPACKER_TARGETS
  unpacker::common_unpacking
  unpacker::common_data_products
  unpacker::nalu_unpacking
  unpacker::nalu_data_products
)

set(ALL_UNPACKER_INCLUDE_DIRS "")
foreach(target IN LISTS UNPACKER_TARGETS)
  get_target_property(tmp_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND ALL_UNPACKER_INCLUDE_DIRS ${tmp_dirs})
endforeach()

list(REMOVE_DUPLICATES ALL_UNPACKER_INCLUDE_DIRS)

# Convert to -I flags
set(UNPACKER_INCLUDE_FLAGS "")
foreach(dir IN LISTS ALL_UNPACKER_INCLUDE_DIRS)
  list(APPEND UNPACKER_INCLUDE_FLAGS "-I${dir}")
endforeach()

list(APPEND ALL_STAGE_HEADERS ${UNPACKER_HEADERS})

# Prepare include flags for MIDAS and unpacker for ROOT dictionary generation
set(MIDAS_INCLUDE_FLAGS "")
foreach(dir IN LISTS MIDASSYS_INCLUDE_DIRS)
  list(APPEND MIDAS_INCLUDE_FLAGS "-I${dir}")
endforeach()

# ROOT dictionary generation with ALL headers and include dirs
ROOT_GENERATE_DICTIONARY(G__Stages
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/stages/LinkDef.h
  OPTIONS
    -I${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MIDAS_INCLUDE_FLAGS}
    ${UNPACKER_INCLUDE_FLAGS}
    -Wno-inconsistent-missing-override
)

# Define the shared library
add_library(analysis_pipeline_stages SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__Stages.cxx
)

add_library(AnalysisPipelineStages::analysis_pipeline_stages ALIAS analysis_pipeline_stages)

set_target_properties(analysis_pipeline_stages PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_compile_features(analysis_pipeline_stages PUBLIC cxx_std_17)

# Include directories for compiling the library
target_include_directories(analysis_pipeline_stages PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages>
  $<INSTALL_INTERFACE:include/midas>
)

foreach(dir IN LISTS MIDASSYS_INCLUDE_DIRS)
  target_include_directories(analysis_pipeline_stages PUBLIC $<BUILD_INTERFACE:${dir}>)
endforeach()

# --- FORCE linking of unpacker libs with -Wl,--no-as-needed to keep them ---
target_link_libraries(analysis_pipeline_stages PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  unpacker::common_data_products
  unpacker::common_unpacking
  unpacker::nalu_data_products
  unpacker::nalu_unpacking
  ${MIDASSYS_LIB_DIR}/libmfe.a
  ${MIDASSYS_LIB_DIR}/libmidas.a
)

target_link_options(analysis_pipeline_stages PUBLIC "-Wl,--no-as-needed")

# --- Set RPATH for runtime dynamic loader to find unpacker libs ---
set_target_properties(analysis_pipeline_stages PROPERTIES
  BUILD_WITH_INSTALL_RPATH TRUE
  INSTALL_RPATH "$ORIGIN/../" # Adjust path as needed to point to unpacker libs location relative to this lib
  INSTALL_RPATH_USE_LINK_PATH TRUE
)

# === Installation section ===
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)

  install(TARGETS analysis_pipeline_stages
    EXPORT AnalysisPipelineStagesTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_stages
  )

  # ROOT dictionary files
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libStages.rootmap
    ${CMAKE_CURRENT_BINARY_DIR}/libStages_rdict.pcm
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  # Export targets
  install(EXPORT AnalysisPipelineStagesTargets
    FILE AnalysisPipelineStagesTargets.cmake
    NAMESPACE AnalysisPipelineStages::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  # Config and version files
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineStagesConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineStages
  )

  export(PACKAGE AnalysisPipelineStages)

endif()
