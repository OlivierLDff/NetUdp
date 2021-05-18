#
#   NetUdp CMake build script
#
#   Copyright Olivier Le Doeuff 2019
#

# ┌──────────────────────────────────────────────────────────────────┐
# │                       CMAKE PROPERTIES                           │
# └──────────────────────────────────────────────────────────────────┘

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ┌──────────────────────────────────────────────────────────────────┐
# │                       PROJECT SETTINGS                           │
# └──────────────────────────────────────────────────────────────────┘

# OPTIONS

set(NETUDP_VERSION_MAJOR 1)
set(NETUDP_VERSION_MINOR 3)
set(NETUDP_VERSION_PATCH 0)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  EXECUTE_PROCESS(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE NETUDP_VERSION_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
if(NOT NETUDP_VERSION_TAG)
  set(NETUDP_VERSION_TAG 00000000)
endif(NOT NETUDP_VERSION_TAG)
set(NETUDP_VERSION_TAG_HEX 0x${NETUDP_VERSION_TAG})
set(NETUDP_VERSION ${NETUDP_VERSION_MAJOR}.${NETUDP_VERSION_MINOR}.${NETUDP_VERSION_PATCH} CACHE STRING "" FORCE)

# General
set(NETUDP_PROJECT "NetUdp" CACHE STRING "Project Name")
set(NETUDP_TARGET ${NETUDP_PROJECT} CACHE STRING "NetUdp library name")
set(NETUDP_VERSION ${NETUDP_VERSION_MAJOR}.${NETUDP_VERSION_MINOR}.${NETUDP_VERSION_PATCH} CACHE STRING "NetUdp current version, this is only decorative and will not configure any files" FORCE)

set(NETUDP_BUILD_SHARED OFF CACHE BOOL "Build as a shared library (ON) or as static (OFF)")
set(NETUDP_FOLDER_PREFIX ${NETUDP_PROJECT} CACHE STRING "Prefix folder for all NetUdp generated targets in generated project (only decorative)")

set(NETUDP_ENABLE_QML ON CACHE BOOL "Embedded Debug Qml module for NetUdp class")
set(NETUDP_ENABLE_PCH ON CACHE BOOL
 "Enable precompile headers support for 'NetUdp'. \"
 Only work if CMake support 'target_precompile_headers'. \"
 This can speed up compilation time.")

# Extra
set(NETUDP_ENABLE_EXAMPLES OFF CACHE BOOL "Create NetUdp examples")
set(NETUDP_ENABLE_TESTS OFF CACHE BOOL "Create test target for NetUdp")

# CREATE PROJECT

project(${NETUDP_PROJECT} VERSION ${NETUDP_VERSION} LANGUAGES C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS "---------------- NETUDP OPTIONS. ----------------")

#General
message(STATUS "NETUDP_PROJECT            : " ${NETUDP_PROJECT})
message(STATUS "NETUDP_TARGET             : " ${NETUDP_TARGET})
message(STATUS "NETUDP_VERSION            : " ${NETUDP_VERSION})
message(STATUS "NETUDP_VERSION_TAG        : " ${NETUDP_VERSION_TAG})
message(STATUS "NETUDP_BUILD_SHARED       : " ${NETUDP_BUILD_SHARED})
message(STATUS "NETUDP_FOLDER_PREFIX      : " ${NETUDP_FOLDER_PREFIX})
message(STATUS "NETUDP_ENABLE_QML         : " ${NETUDP_ENABLE_QML})
message(STATUS "NETUDP_ENABLE_PCH         : " ${NETUDP_ENABLE_PCH})
message(STATUS "NETUDP_ENABLE_EXAMPLES    : " ${NETUDP_ENABLE_EXAMPLES})
message(STATUS "NETUDP_ENABLE_TESTS       : " ${NETUDP_ENABLE_TESTS})

message(STATUS "---------------- DONE WITH OPTIONS. -----------------")

# ┌──────────────────────────────────────────────────────────────────┐
# │                       DEPENDENCIES                               │
# └──────────────────────────────────────────────────────────────────┘

find_package(Qt5 REQUIRED COMPONENTS
  Core
  Qml
  Network
)

if(NETUDP_ENABLE_QML)
  find_package(Qt5 REQUIRED COMPONENTS QuickCompiler)
endif()

include(cmake/FetchSpdlog.cmake)
include(cmake/FetchRecyler.cmake)

# ┌──────────────────────────────────────────────────────────────────┐
# │                          NETUDP                                  │
# └──────────────────────────────────────────────────────────────────┘

if(NETUDP_ENABLE_QML)
  file(GLOB_RECURSE NETUDP_QML_SRCS qml/*.qml)

  foreach(SOURCE IN ITEMS ${NETUDP_QML_SRCS})
    get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
    file(RELATIVE_PATH SOURCE_PATH_REL ${CMAKE_CURRENT_SOURCE_DIR}/qml "${SOURCE_PATH}")
    string(REPLACE "/" "\\" GROUP_PATH "${SOURCE_PATH_REL}")
    source_group("Qml\\${GROUP_PATH}" FILES "${SOURCE}")
  endforeach()

  # Add qml to the qtquick compiler
  qtquick_compiler_add_resources(NETUDP_QML_RES ${CMAKE_CURRENT_SOURCE_DIR}/qml/NetUdp.qrc)

  # Trick to rerun cmake each time NetUdp.qrc is updated
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/NetUdp.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/qml.junk
    )

  # Dont Run Moc on cpp file generated from qml
  foreach(NETUDP_CURRENT_QML_FILE ${NETUDP_QML_RES})
    set_property(SOURCE ${NETUDP_CURRENT_QML_FILE} PROPERTY SKIP_AUTOMOC ON)
    source_group("Qml\\QtQuickCompiler Files" FILES ${NETUDP_CURRENT_QML_FILE})
  endforeach()
endif()
# NETUDP_ENABLE_QML

set(NETUDP_SRCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NETUDP_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(NETUDP_PRIVATE_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/include/Net/Udp)

# NETUDP SOURCES
set(NETUDP_UTILS_SRCS
  ${NETUDP_SRCS_FOLDER}/Utils.cpp
  ${NETUDP_SRCS_FOLDER}/Logger.cpp
  ${NETUDP_SRCS_FOLDER}/Version.cpp
  ${NETUDP_SRCS_FOLDER}/InterfacesProvider.cpp
  )

set(NETUDP_API_SRCS
  ${NETUDP_SRCS_FOLDER}/Datagram.cpp
  ${NETUDP_SRCS_FOLDER}/RecycledDatagram.cpp
  ${NETUDP_SRCS_FOLDER}/Socket.cpp
  ${NETUDP_SRCS_FOLDER}/Worker.cpp
  )

set(NETUDP_SRCS ${NETUDP_UTILS_SRCS}
  ${NETUDP_API_SRCS}
  ${NETUDP_QML_SRCS}
  ${NETUDP_QML_RES}
  )

set(NETUDP_UTILS_INCS
  ${NETUDP_PRIVATE_INCS_FOLDER}/NetUdp.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Export.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Property.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Logger.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Utils.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Version.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/InterfacesProvider.hpp
  )

set(NETUDP_API_INCS
  ${NETUDP_PRIVATE_INCS_FOLDER}/Datagram.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/RecycledDatagram.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Socket.hpp
  ${NETUDP_PRIVATE_INCS_FOLDER}/Worker.hpp
  )

set(NETUDP_INCS ${NETUDP_UTILS_INCS}
  ${NETUDP_API_INCS}
  )

source_group("Udp\\Utils" FILES ${NETUDP_UTILS_INCS})
source_group("Udp" FILES ${NETUDP_API_INCS})

source_group("Udp\\Utils" FILES ${NETUDP_UTILS_SRCS})
source_group("Udp" FILES ${NETUDP_API_SRCS})

if(NETUDP_BUILD_SHARED)

  add_library(${NETUDP_TARGET} SHARED ${NETUDP_SRCS} ${NETUDP_INCS})
  target_compile_definitions(${NETUDP_TARGET} PRIVATE "-DNETUDP_DLL_EXPORT")

else()

  add_library(${NETUDP_TARGET} STATIC  ${NETUDP_SRCS} ${NETUDP_INCS})
  target_compile_definitions(${NETUDP_TARGET} PUBLIC "-DNETUDP_STATIC")

endif()

target_compile_features(${NETUDP_TARGET} PUBLIC cxx_std_17)
target_include_directories(${NETUDP_TARGET} PUBLIC $<BUILD_INTERFACE:${NETUDP_INCS_FOLDER}>)

target_link_libraries(${NETUDP_TARGET} PUBLIC
  Recycler
  spdlog
  Qt5::Core
  Qt5::Qml
  Qt5::Network
  )

set_target_properties(${NETUDP_TARGET} PROPERTIES AUTOMOC TRUE)
if(NETUDP_ENABLE_QML)
  set_target_properties(${NETUDP_TARGET} PROPERTIES AUTORCC TRUE)
  target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_ENABLE_QML)
endif()

set_target_properties(${NETUDP_TARGET} PROPERTIES FOLDER ${NETUDP_FOLDER_PREFIX})

target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_MAJOR=${NETUDP_VERSION_MAJOR})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_MINOR=${NETUDP_VERSION_MINOR})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_PATCH=${NETUDP_VERSION_PATCH})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_TAG=${NETUDP_VERSION_TAG})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_TAG_HEX=${NETUDP_VERSION_TAG_HEX})

if(NETUDP_ENABLE_PCH AND COMMAND target_precompile_headers)
  target_precompile_headers(${NETUDP_TARGET} PRIVATE ${NETUDP_PRIVATE_INCS_FOLDER}/Pch/Pch.hpp)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │                         EXAMPLES                                 │
# └──────────────────────────────────────────────────────────────────┘

if(NETUDP_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()


# ┌──────────────────────────────────────────────────────────────────┐
# │                         TESTS                                    │
# └──────────────────────────────────────────────────────────────────┘

if(NETUDP_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
