include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

set(RECYCLER_REPOSITORY
    "https://github.com/OlivierLDff/Recycler.git"
    CACHE STRING "Recycler repository url"
)
set(RECYCLER_TAG
    "v1.3.4"
    CACHE STRING "Recycler git tag"
)

CPMAddPackage(
  NAME Recycler
  GIT_REPOSITORY ${RECYCLER_REPOSITORY}
  GIT_TAG ${RECYCLER_TAG}
)
