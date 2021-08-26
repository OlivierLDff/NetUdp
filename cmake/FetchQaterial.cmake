include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

set(QATERIAL_REPOSITORY
    "https://github.com/OlivierLDff/Qaterial.git"
    CACHE STRING "Qaterial repository url"
)
set(QATERIAL_TAG
    "v1.4.5"
    CACHE STRING "Qaterial git tag"
)

CPMAddPackage(
  NAME Qaterial
  GIT_REPOSITORY ${QATERIAL_REPOSITORY}
  GIT_TAG ${QATERIAL_TAG}
  OPTIONS "QATERIAL_FOLDER_PREFIX Dependencies"
)
