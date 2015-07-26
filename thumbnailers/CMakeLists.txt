project(kdegraphics-thumbnailers)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

find_package(KDE4 4.14.3 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(${KDE4_INCLUDES})

macro_optional_find_package(Kexiv2)
macro_optional_find_package(Kdcraw)
macro_optional_find_package(GettextPO)

macro_log_feature(KEXIV2_FOUND "libkexiv2" "A library for reading and writing image metadata" "www.kde.org" FALSE "" "Required to build the RAW thumbnailer")
macro_log_feature(KDCRAW_FOUND "libkdcraw" "A library for accessing raw files" "www.kde.org" FALSE "" "Required to build the RAW thumbnailer")
macro_log_feature(GETTEXTPO_FOUND "gettext-po" "A library for processing PO files" "www.gnu.org/software/gettext" FALSE "" "Required to build the PO thumbnailer")

macro_optional_add_subdirectory(audio)

macro_optional_add_subdirectory(ps)

if(KEXIV2_FOUND AND KDCRAW_FOUND)
    macro_optional_add_subdirectory(raw)
endif()

if(GETTEXTPO_FOUND)
    macro_optional_add_subdirectory(po)
endif(GETTEXTPO_FOUND)

