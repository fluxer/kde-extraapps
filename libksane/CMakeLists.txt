PROJECT(libksane)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

# Check if sane API is available.
find_package(Sane)
set_package_properties(Sane PROPERTIES
    DESCRIPTION "Scanner Access Now Easy (SANE) development package"
    URL "http://www.sane-project.org"
    PURPOSE "Needed by libksane"
    TYPE REQUIRED
)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/libksane
)

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0
# 0.1.1 => 0.1.1
# 0.2.0 => 0.2.0

# Library API version
SET(KSANE_LIB_MAJOR_VERSION "0")
SET(KSANE_LIB_MINOR_VERSION "3")
SET(KSANE_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KSANE_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KSANE_LIB_SO_CUR_VERSION "0")
SET(KSANE_LIB_SO_REV_VERSION "2")
SET(KSANE_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

SET(KSANE_LIB_VERSION_STRING "${KSANE_LIB_MAJOR_VERSION}.${KSANE_LIB_MINOR_VERSION}.${KSANE_LIB_PATCH_VERSION}${KSANE_LIB_SUFFIX_VERSION}")
SET(KSANE_LIB_VERSION_ID "0x0${KSANE_LIB_MAJOR_VERSION}0${KSANE_LIB_MINOR_VERSION}0${KSANE_LIB_PATCH_VERSION}")

SET(KSANE_LIB_SO_VERSION_STRING "${KSANE_LIB_SO_CUR_VERSION}.${KSANE_LIB_SO_REV_VERSION}.${KSANE_LIB_SO_AGE_VERSION}")

# =======================================================

ADD_DEFINITIONS(-DKDE_DEFAULT_DEBUG_AREA=51004) 

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/libksane/version.h.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libksane/version.h
)

# =======================================================

ADD_SUBDIRECTORY(libksane)
ADD_SUBDIRECTORY(ksaneplugin)
