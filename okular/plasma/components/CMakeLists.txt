add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

set(okular_SRCS
    okularplugin.cpp
    ${PROJECT_SOURCE_DIR}/ui/pagepainter.cpp
    ${PROJECT_SOURCE_DIR}/ui/guiutils.cpp
    ${PROJECT_SOURCE_DIR}/ui/tocmodel.cpp
    pageitem.cpp
    documentitem.cpp
    thumbnailitem.cpp
)
    
kde4_add_kcfg_files(okular_SRCS ${PROJECT_SOURCE_DIR}/conf/settings.kcfgc )

add_library(okularplugin SHARED ${okular_SRCS})
target_link_libraries(okularplugin
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTXML_LIBRARY}
    okularcore
)

install(TARGETS okularplugin DESTINATION ${KDE4_IMPORTS_INSTALL_DIR}/org/kde/okular)
install(FILES qmldir DESTINATION ${KDE4_IMPORTS_INSTALL_DIR}/org/kde/okular)

#add_subdirectory(test)
