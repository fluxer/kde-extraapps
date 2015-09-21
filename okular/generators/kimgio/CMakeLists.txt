include_directories(
   ${CMAKE__SOURCE_DIR}/../..
   ${CMAKE_BINARY_DIR}
   ${QIMAGEBLITZ_INCLUDES}
   ${KEXIV2_INCLUDE_DIR}
)


########### next target ###############

set(okularGenerator_kimgio_PART_SRCS generator_kimgio.cpp )


kde4_add_plugin(okularGenerator_kimgio ${okularGenerator_kimgio_PART_SRCS})
target_link_libraries(okularGenerator_kimgio okularcore ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ${KEXIV2_LIBRARIES} )
install(TARGETS okularGenerator_kimgio DESTINATION ${PLUGIN_INSTALL_DIR})

if(ENABLE_TESTING)
    set( kimgiotest_SRCS tests/kimgiotest.cpp
        ${PROJECT_SOURCE_DIR}/ui/pagepainter.cpp
        ${PROJECT_SOURCE_DIR}/ui/guiutils.cpp
    )
    kde4_add_kcfg_files(kimgiotest_SRCS ${PROJECT_SOURCE_DIR}/conf/settings.kcfgc )
    kde4_add_test( kimgiotest ${kimgiotest_SRCS})
    target_link_libraries(kimgiotest
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KIO_LIBS}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        ${QIMAGEBLITZ_LIBRARIES}
        okularcore
    )
endif()

########### install files ###############

install( FILES libokularGenerator_kimgio.desktop okularKimgio.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_kimgio.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

#install( FILES gui.rc DESTINATION ${DATA_INSTALL_DIR}/okular_kimgio )
