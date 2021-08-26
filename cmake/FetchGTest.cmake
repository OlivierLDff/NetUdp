include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

set(GTEST_REPOSITORY
    "https://github.com/OlivierLDff/googletest"
    CACHE STRING "googletest repository url"
)
set(GTEST_TAG
    "release-1.11.0"
    CACHE STRING "googletest git tag"
)

CPMAddPackage(
  NAME googletest
  GIT_REPOSITORY ${GTEST_REPOSITORY}
  GIT_TAG ${GTEST_TAG}
  OPTIONS "gtest_force_shared_crt ON"
  OPTIONS "BUILD_GMOCK OFF"
)

if(TARGET gtest)
  set_target_properties(gtest PROPERTIES FOLDER "Dependencies/gtest")
endif()

if(TARGET gtest_main)
  set_target_properties(gtest_main PROPERTIES FOLDER "Dependencies/gtest")
endif()

if(TARGET gmock)
  set_target_properties(gmock PROPERTIES FOLDER "Dependencies/gtest")
endif()

if(TARGET gmock_main)
  set_target_properties(gmock_main PROPERTIES FOLDER "Dependencies/gtest")
endif()
