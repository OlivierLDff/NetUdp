#
# NetUdp CTest script
#
# Copyright Olivier Le Doeuff 2020

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Quick QuickTest)
include(${PROJECT_SOURCE_DIR}/cmake/FetchGTest.cmake)

set(NETUDP_TEST_TARGET NetUdp_Tests)
set(NETUDP_QUICK_TEST_TARGET NetUdp_QuickTests)

message(STATUS "Add Test: ${NETUDP_TEST_TARGET}")

# Create the executable
add_executable(${NETUDP_TEST_TARGET} Tests.cpp)
target_include_directories(${NETUDP_TEST_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${NETUDP_TEST_TARGET} PRIVATE ${NETUDP_TARGET} gtest gmock Qt${QT_VERSION_MAJOR}::Test)
set_target_properties(${NETUDP_TEST_TARGET} PROPERTIES AUTOMOC TRUE)
set_target_properties(${NETUDP_TEST_TARGET} PROPERTIES FOLDER ${NETUDP_FOLDER_PREFIX}/Tests)

# Register the test in ctest
add_test(NAME ${NETUDP_TEST_TARGET} COMMAND ${NETUDP_TEST_TARGET})

add_executable(${NETUDP_QUICK_TEST_TARGET} QuickTests.cpp)
set_target_properties(${NETUDP_QUICK_TEST_TARGET} PROPERTIES AUTOMOC TRUE AUTORCC TRUE)
target_link_libraries(${NETUDP_QUICK_TEST_TARGET} PRIVATE ${NETUDP_TARGET} Qt${QT_VERSION_MAJOR}::QuickTest Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Quick)
set_target_properties(${NETUDP_QUICK_TEST_TARGET} PROPERTIES FOLDER ${NETUDP_FOLDER_PREFIX}/Tests)

file(GLOB QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.qml)
foreach(QML_FILE ${QML_FILES})
  get_filename_component(QML_FILE_NAME ${QML_FILE} NAME_WE)
  message(STATUS "Add Test: ${NETUDP_QUICK_TEST_TARGET}_${QML_FILE_NAME}")
  add_test(NAME ${NETUDP_QUICK_TEST_TARGET}_${QML_FILE_NAME} COMMAND "$<TARGET_FILE:${NETUDP_QUICK_TEST_TARGET}>" -input "${QML_FILE}")
endforeach()

if(NETUDP_ENABLE_PCH AND COMMAND target_precompile_headers)
  target_precompile_headers(${NETUDP_TEST_TARGET} PRIVATE ${NETUDP_SRCS_FOLDER}/NetUdp/Pch/Pch.hpp)
  target_precompile_headers(${NETUDP_QUICK_TEST_TARGET} PRIVATE ${NETUDP_SRCS_FOLDER}/NetUdp/Pch/Pch.hpp)
endif()
