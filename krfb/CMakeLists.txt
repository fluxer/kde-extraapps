project(krfb)

find_package(LibVNCServer REQUIRED)

macro_optional_find_package(TelepathyQt4)
macro_log_feature(TelepathyQt4_FOUND "telepathy-qt" "Telepathy Qt Bindings" "http://telepathy.freedesktop.org" FALSE "0.9" "Needed to build Telepathy Tubes support.")

macro_optional_find_package(KTp)
macro_log_feature(KTP_FOUND "KTP" "KDE Telepathy" "https://projects.kde.org/projects/extragear/network/telepathy" FALSE "" "Needed to build KDE IM Contacts Display in KRFB.")

macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE)
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/krfb
    ${CMAKE_CURRENT_SOURCE_DIR}/krfb
    ${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui
)

if(Q_WS_X11)
    if(NOT X11_XTest_FOUND)
        message(FATAL_ERROR "krfb requires the libXtst (http://xorg.freedesktop.org) to be built")
    endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

add_subdirectory(krfb)
add_subdirectory(framebuffers)

