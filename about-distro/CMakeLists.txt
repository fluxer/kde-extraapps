project(kcm-about-distro)

set(MAJOR_VERSION "1")
set(MINOR_VERSION "1")
set(PATCH_VERSION "0")
set(SUFFIX_VERSION "")

set(VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
set(VERSION_STRING_FULL "${VERSION_STRING}")

add_definitions(-std=c++0x)

add_subdirectory(src)

