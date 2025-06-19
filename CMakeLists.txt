cmake_minimum_required(VERSION 3.14)

project(AnalysisPipelineMidas VERSION 0.1 LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# Find dependencies (must be installed in system or user environment)
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

include(${ROOT_USE_FILE})

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE ALL_STAGE_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
list(FILTER ALL_STAGE_HEADERS EXCLUDE REGEX "LinkDef\\.h$")

set(MIDAS_INCLUDE_FLAGS "")
foreach(dir IN LISTS MIDASSYS_INCLUDE_DIRS)
  list(APPEND MIDAS_INCLUDE_FLAGS "-I${dir}")
endforeach()

ROOT_GENERATE_DICTIONARY(G__Midas
  ${ALL_STAGE_HEADERS}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/LinkDef.h
  OPTIONS
    -I${CMAKE_CURRENT_SOURCE_DIR}/include
    ${MIDAS_INCLUDE_FLAGS}
    -Wno-inconsistent-missing-override
)

add_library(analysis_pipeline_midas SHARED
  ${SRC_FILES}
  ${CMAKE_CURRENT_BINARY_DIR}/G__Midas.cxx
)

set_target_properties(analysis_pipeline_midas PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 0
)

target_compile_features(analysis_pipeline_midas PUBLIC cxx_std_17)

foreach(dir IN LISTS MIDASSYS_INCLUDE_DIRS)
  target_include_directories(analysis_pipeline_midas PUBLIC
    $<BUILD_INTERFACE:${dir}>
  )
endforeach()

target_include_directories(analysis_pipeline_midas PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_midas>
  $<INSTALL_INTERFACE:include/midas>
)

target_link_libraries(analysis_pipeline_midas PUBLIC
  ROOT::Core ROOT::RIO ROOT::Tree
  spdlog::spdlog
  nlohmann_json::nlohmann_json
  ${MIDASSYS_LIB_DIR}/libmfe.a
  ${MIDASSYS_LIB_DIR}/libmidas.a
)

add_library(AnalysisPipelineMidas::analysis_pipeline_midas ALIAS analysis_pipeline_midas)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  # Install only YOUR own targets and headers

  install(TARGETS analysis_pipeline_midas
    EXPORT AnalysisPipelineMidasTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/analysis_pipeline_midas
  )

  install(EXPORT AnalysisPipelineMidasTargets
    FILE AnalysisPipelineMidasTargets.cmake
    NAMESPACE AnalysisPipelineMidas::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineMidas
    EXPORT_LINK_INTERFACE_LIBRARIES
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineMidasConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineMidasConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineMidas
  )

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineMidasConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineMidas
  )
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AnalysisPipelineMidasConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AnalysisPipelineMidas
  )

  export(PACKAGE AnalysisPipelineMidas)
endif()
