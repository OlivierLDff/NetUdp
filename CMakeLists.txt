# ~~~
# Copyright 2019 - 2021 Olivier Le Doeuff
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ~~~

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ──── Include guards ────

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. " "Please make a new directory (called a build directory) and run CMake from there.")
endif()

# ──── Main Project behavior ────

set(NETUDP_MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(NETUDP_MAIN_PROJECT ON)
endif()

# ──── Default build to Release ────

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose Release or Debug" FORCE
  )
endif()

# ───── 🔧 Configuration ─────

set(NETUDP_VERSION_MAJOR 1)
set(NETUDP_VERSION_MINOR 3)
set(NETUDP_VERSION_PATCH 1)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
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
set(NETUDP_VERSION
    ${NETUDP_VERSION_MAJOR}.${NETUDP_VERSION_MINOR}.${NETUDP_VERSION_PATCH}
    CACHE STRING "" FORCE
)

# General
set(NETUDP_PROJECT
    "NetUdp"
    CACHE STRING "Project Name"
)
set(NETUDP_TARGET
    ${NETUDP_PROJECT}
    CACHE STRING "NetUdp library name"
)
set(NETUDP_VERSION
    ${NETUDP_VERSION_MAJOR}.${NETUDP_VERSION_MINOR}.${NETUDP_VERSION_PATCH}
    CACHE STRING "NetUdp current version, this is only decorative and will not configure any files" FORCE
)

set(NETUDP_BUILD_SHARED
    OFF
    CACHE BOOL "Build as a shared library (ON) or as static (OFF)"
)
set(NETUDP_FOLDER_PREFIX
    ${NETUDP_PROJECT}
    CACHE STRING "Prefix folder for all NetUdp generated targets in generated project (only decorative)"
)

set(NETUDP_ENABLE_QML
    ON
    CACHE BOOL "Embedded Debug Qml module for NetUdp class"
)
set(NETUDP_ENABLE_PCH
    ON
    CACHE BOOL "Enable precompile headers support for 'NetUdp'. \"
 Only work if CMake support 'target_precompile_headers'. \"
 This can speed up compilation time."
)
set(NETUDP_ENABLE_UNITY_BUILD
    ON
    CACHE BOOL "NetUdp source files will be combined into batches for faster compilation."
)

# Extra
set(NETUDP_ENABLE_EXAMPLES
    OFF
    CACHE BOOL "Create NetUdp examples"
)
set(NETUDP_ENABLE_TESTS
    OFF
    CACHE BOOL "Create test target for NetUdp"
)
set(NETUDP_VERBOSE
    ${NETUDP_MAIN_PROJECT}
    CACHE BOOL "Verbose cmake configuration"
)

# CREATE PROJECT

project(
  ${NETUDP_PROJECT}
  VERSION ${NETUDP_VERSION}
  LANGUAGES C CXX
)
if(NETUDP_MAIN_PROJECT)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if(NETUDP_VERBOSE)
  message(STATUS "---------------- NETUDP OPTIONS. ----------------")
  message(STATUS "NETUDP_PROJECT            : " ${NETUDP_PROJECT})
  message(STATUS "NETUDP_TARGET             : " ${NETUDP_TARGET})
  message(STATUS "NETUDP_VERSION            : " ${NETUDP_VERSION})
  message(STATUS "NETUDP_VERSION_TAG        : " ${NETUDP_VERSION_TAG})
  message(STATUS "NETUDP_BUILD_SHARED       : " ${NETUDP_BUILD_SHARED})
  message(STATUS "NETUDP_FOLDER_PREFIX      : " ${NETUDP_FOLDER_PREFIX})
  message(STATUS "NETUDP_ENABLE_QML         : " ${NETUDP_ENABLE_QML})
  message(STATUS "NETUDP_ENABLE_PCH         : " ${NETUDP_ENABLE_PCH})
  message(STATUS "NETUDP_ENABLE_UNITY_BUILD : ${NETUDP_ENABLE_UNITY_BUILD}")
  message(STATUS "NETUDP_ENABLE_EXAMPLES    : " ${NETUDP_ENABLE_EXAMPLES})
  message(STATUS "NETUDP_ENABLE_TESTS       : " ${NETUDP_ENABLE_TESTS})
  message(STATUS "---------------- DONE WITH OPTIONS. -----------------")
endif()

# ───── Add dependencies ─────

find_package(
  QT NAMES Qt6 Qt5
  COMPONENTS Core
  REQUIRED
)

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Core Qml Network CONFIG
  REQUIRED
)

find_package(Qt5 REQUIRED COMPONENTS Core Qml Network)

if(NETUDP_ENABLE_QML)
  find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Quick QuickCompiler CONFIG
    REQUIRED
  )
endif()

include(cmake/FetchSpdlog.cmake)
include(cmake/FetchRecyler.cmake)

# NetUdp

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
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qml/NetUdp.qrc ${CMAKE_CURRENT_BINARY_DIR}/qml.junk)

  # Dont Run Moc on cpp file generated from qml
  foreach(NETUDP_CURRENT_QML_FILE ${NETUDP_QML_RES})
    set_property(SOURCE ${NETUDP_CURRENT_QML_FILE} PROPERTY SKIP_AUTOMOC ON)
    source_group("Qml\\QtQuickCompiler Files" FILES ${NETUDP_CURRENT_QML_FILE})
  endforeach()
endif()

set(NETUDP_SRCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NETUDP_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(NETUDP_SRCS
    ${NETUDP_INCS_FOLDER}/NetUdp/NetUdp.hpp
    ${NETUDP_INCS_FOLDER}/NetUdp/Export.hpp
    ${NETUDP_INCS_FOLDER}/NetUdp/Utils.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Utils.cpp
    ${NETUDP_INCS_FOLDER}/NetUdp/Logger.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Logger.cpp
    ${NETUDP_INCS_FOLDER}/NetUdp/Version.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Version.cpp
    ${NETUDP_INCS_FOLDER}/NetUdp/InterfacesProvider.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/InterfacesProvider.cpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Datagram.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Datagram.cpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/RecycledDatagram.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/RecycledDatagram.cpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Socket.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Socket.cpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Worker.hpp
    ${NETUDP_SRCS_FOLDER}/NetUdp/Worker.cpp
)

source_group(TREE "${NETUDP_SRCS_FOLDER}/" FILES ${NETUDP_SRCS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/" FILES ${NETUDP_QML_SRCS})

set(NETUDP_SRCS ${NETUDP_SRCS} ${NETUDP_QML_RES} ${NETUDP_QML_SRCS})

if(NETUDP_BUILD_SHARED)

  add_library(${NETUDP_TARGET} SHARED ${NETUDP_SRCS})
  target_compile_definitions(${NETUDP_TARGET} PRIVATE "-DNETUDP_DLL_EXPORT")

else()

  add_library(${NETUDP_TARGET} STATIC ${NETUDP_SRCS})
  target_compile_definitions(${NETUDP_TARGET} PUBLIC "-DNETUDP_STATIC")

endif()

target_compile_features(${NETUDP_TARGET} PUBLIC cxx_std_17)
target_include_directories(${NETUDP_TARGET} PUBLIC $<BUILD_INTERFACE:${NETUDP_INCS_FOLDER}>)
set_target_properties(${NETUDP_TARGET} PROPERTIES UNITY_BUILD ${NETUDP_ENABLE_UNITY_BUILD})

target_link_libraries(
  ${NETUDP_TARGET}
  PUBLIC Recycler
  PUBLIC spdlog
  PUBLIC Qt${QT_VERSION_MAJOR}::Core
  PUBLIC Qt${QT_VERSION_MAJOR}::Qml
  PUBLIC Qt${QT_VERSION_MAJOR}::Network
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
  target_precompile_headers(${NETUDP_TARGET} PRIVATE ${NETUDP_SRCS_FOLDER}/NetUdp/Pch/Pch.hpp)
endif()

# ───── 🚀 Add Examples ─────

if(NETUDP_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()

# ───── ✅ Add Tests ─────

if(NETUDP_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ───── 🔊 Log dev command and info ─────

if(NETUDP_MAIN_PROJECT)
  include(cmake/PrintConfiguration.cmake)
endif()
