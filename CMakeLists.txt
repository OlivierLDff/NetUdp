#
#   NetUdp CMake build script
#
#   Copyright Olivier Le Doeuff 2019
#

# ┌──────────────────────────────────────────────────────────────────┐
# │                       CMAKE PROPERTIES                           │
# └──────────────────────────────────────────────────────────────────┘

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# DEPENDANCIES

set(QSUPERMACROS_REPOSITORY "https://github.com/OlivierLDff/QSuperMacros" CACHE STRING "Repository of QSuperMacros")
set(QSUPERMACROS_TAG "master" CACHE STRING "Git tag/branch of QSuperMacros")

# ┌──────────────────────────────────────────────────────────────────┐
# │                       PROJECT SETTINGS                           │
# └──────────────────────────────────────────────────────────────────┘

# OPTIONS

set(NETUDP_VERSION_MAJOR 1)
set(NETUDP_VERSION_MINOR 0)
set(NETUDP_VERSION_PATCH 0)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    EXECUTE_PROCESS(
        COMMAND git describe --always
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

set(NETUDP_USE_NAMESPACE ON CACHE BOOL "Should we use namespace")
set(NETUDP_NAMESPACE ${NETUDP_PROJECT} CACHE STRING "Namespace for the library")

set(NETUDP_ENABLE_QML ON CACHE BOOL "Embedded Debug Qml module for NetUdp class")

# Install
set(NETUDP_ENABLE_INSTALL OFF CACHE BOOL "Create or not a target for install")
set(NETUDP_INSTALL_PREFIX ${NETUDP_PROJECT} CACHE STRING "Prefix for all NetUdp headers in the install folder")

# CREATE PROJECT

project(${NETUDP_PROJECT} VERSION ${NETUDP_VERSION} LANGUAGES C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 14)

# LOG OPTIONS

message(STATUS "---------------- NETUDP OPTIONS. ----------------")

#General
message(STATUS "NETUDP_PROJECT            : " ${NETUDP_PROJECT})
message(STATUS "NETUDP_TARGET             : " ${NETUDP_TARGET})
message(STATUS "NETUDP_VERSION            : " ${NETUDP_VERSION})
message(STATUS "NETUDP_VERSION_TAG        : " ${NETUDP_VERSION_TAG})
message(STATUS "NETUDP_BUILD_SHARED       : " ${NETUDP_BUILD_SHARED})
message(STATUS "NETUDP_FOLDER_PREFIX      : " ${NETUDP_FOLDER_PREFIX})
message(STATUS "NETUDP_ENABLE_QML         : " ${NETUDP_ENABLE_QML})

# Install
message(STATUS "NETUDP_ENABLE_INSTALL     : " ${NETUDP_ENABLE_INSTALL})
if(NETUDP_ENABLE_INSTALL)
message(STATUS "NETUDP_INSTALL_PREFIX     : " ${NETUDP_INSTALL_PREFIX})
endif(NETUDP_ENABLE_INSTALL)

message(STATUS "---------------- DONE WITH OPTIONS. -----------------")

# ┌──────────────────────────────────────────────────────────────────┐
# │                         QT5 CMAKE                                │
# └──────────────────────────────────────────────────────────────────┘

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)

if(NETUDP_ENABLE_QML)
    find_package(Qt5QuickCompiler CONFIG REQUIRED)
endif() # NETUDP_ENABLE_QML

# ┌──────────────────────────────────────────────────────────────────┐
# │                       DEPENDENCIES                               │
# └──────────────────────────────────────────────────────────────────┘

include(FetchContent)

# QSuperMacros
FetchContent_Declare(
    QSuperMacros
    GIT_REPOSITORY ${QSUPERMACROS_REPOSITORY}
    GIT_TAG        ${QSUPERMACROS_TAG}
)

set(QSUPERMACROS_FOLDER_PREFIX "Dependencies/QSuperMacros" CACHE STRING "")

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET ON)

# Make all dependencies avaiables
FetchContent_MakeAvailable(QSuperMacros)

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
endif() # NETUDP_ENABLE_QML

set(NETUDP_SRCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NETUDP_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
set(NETUDP_PRIVATE_INCS_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src/include/NetUdp)

# NETUDP SOURCES
set(NETUDP_UTILS_SRCS
    ${NETUDP_SRCS_FOLDER}/Utils.cpp
    ${NETUDP_SRCS_FOLDER}/Version.cpp
)

set(NETUDP_API_SRCS
    ${NETUDP_SRCS_FOLDER}/AbstractServer.cpp
    ${NETUDP_SRCS_FOLDER}/Datagram.cpp
    ${NETUDP_SRCS_FOLDER}/Server.cpp
    ${NETUDP_SRCS_FOLDER}/ServerWorker.cpp
)

set(NETUDP_SRCS ${NETUDP_UTILS_SRCS}
    ${NETUDP_API_SRCS}
    ${NETUDP_QML_SRCS}
    ${NETUDP_QML_RES}
)

set(NETUDP_UTILS_INCS
    ${NETUDP_INCS_FOLDER}/NetUdp.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/Export.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/Utils.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/Version.hpp
)

set(NETUDP_API_INCS
    ${NETUDP_PRIVATE_INCS_FOLDER}/AbstractServer.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/Export.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/Datagram.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/Server.hpp
    ${NETUDP_PRIVATE_INCS_FOLDER}/ServerWorker.hpp
)
# endif(NETUDP_ENABLE_FLATBUFFERS)

set(NETUDP_INCS ${NETUDP_UTILS_INCS}
    ${NETUDP_API_INCS}
)

source_group("Utils" FILES ${NETUDP_UTILS_INCS})
source_group("Api" FILES ${NETUDP_API_INCS})

source_group("Utils" FILES ${NETUDP_UTILS_SRCS})
source_group("Api" FILES ${NETUDP_API_SRCS})

# NETUDP TARGET
if(NETUDP_BUILD_SHARED)

    add_library(${NETUDP_TARGET} SHARED ${NETUDP_SRCS} ${NETUDP_INCS})
    target_compile_definitions(${NETUDP_TARGET} PRIVATE "-DNETUDP_DLL_EXPORT")

else(NETUDP_BUILD_SHARED)

    add_library(${NETUDP_TARGET} STATIC  ${NETUDP_SRCS} ${NETUDP_INCS})
    target_compile_definitions(${NETUDP_TARGET} PUBLIC "-DNETUDP_STATIC")

endif(NETUDP_BUILD_SHARED)

# Set include directory
target_include_directories(${NETUDP_TARGET} PUBLIC $<BUILD_INTERFACE:${NETUDP_INCS_FOLDER}> $<INSTALL_INTERFACE:${NETUDP_INSTALL_PREFIX}>)

# Avoid libnetcode.io on darwin for example.
set_target_properties(${NETUDP_TARGET} PROPERTIES PREFIX "")

# Link libraries
target_link_libraries(${NETUDP_TARGET} ${QSUPERMACROS_TARGET})

set_target_properties(${NETUDP_TARGET} PROPERTIES AUTOMOC TRUE)
if(NETUDP_ENABLE_QML)
    set_target_properties(${NETUDP_TARGET} PROPERTIES AUTORCC TRUE)
endif() # NETUDP_ENABLE_QML
qt5_use_modules(${NETUDP_TARGET} Core Network)

set_target_properties(${NETUDP_TARGET} PROPERTIES FOLDER ${NETUDP_FOLDER_PREFIX})

# Version
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_MAJOR=${NETUDP_VERSION_MAJOR})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_MINOR=${NETUDP_VERSION_MINOR})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_PATCH=${NETUDP_VERSION_PATCH})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_TAG=${NETUDP_VERSION_TAG})
target_compile_definitions(${NETUDP_TARGET} PRIVATE -DNETUDP_VERSION_TAG_HEX=${NETUDP_VERSION_TAG_HEX})

if(NETUDP_USE_NAMESPACE)
    target_compile_definitions(${NETUDP_TARGET} PUBLIC -DNETUDP_USE_NAMESPACE)
    target_compile_definitions(${NETUDP_TARGET} PUBLIC -DNETUDP_NAMESPACE=${NETUDP_NAMESPACE})
endif(NETUDP_USE_NAMESPACE)

# ┌──────────────────────────────────────────────────────────────────┐
# │                           TESTS                                  │
# └──────────────────────────────────────────────────────────────────┘

# ┌──────────────────────────────────────────────────────────────────┐
# │                         EXAMPLES                                 │
# └──────────────────────────────────────────────────────────────────┘

# ┌──────────────────────────────────────────────────────────────────┐
# │                         INSTALL                                  │
# └──────────────────────────────────────────────────────────────────┘

if(NETUDP_ENABLE_INSTALL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CInstallLists.txt)
endif(NETUDP_ENABLE_INSTALL)
