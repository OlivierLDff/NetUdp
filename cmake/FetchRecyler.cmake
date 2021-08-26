include(FetchContent)

set(RECYCLER_REPOSITORY
    "https://github.com/OlivierLDff/Recycler.git"
    CACHE STRING "Recycler repository url"
)
set(RECYCLER_TAG
    "master"
    CACHE STRING "Recycler git tag"
)

FetchContent_Declare(
  Recycler
  GIT_REPOSITORY ${RECYCLER_REPOSITORY}
  GIT_TAG ${RECYCLER_TAG}
)

FetchContent_MakeAvailable(Recycler)
