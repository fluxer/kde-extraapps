project(CharacterRunner)

# avoid putting the generated files in target specific
# directory which is the case when automated
set(CMAKE_AUTOUIC OFF)

# We add our source code here
set(krunner_charrunner_SRCS charrunner.cpp)
set(kcm_krunner_charrunner_SRCS
    charrunner_config.cpp
)
qt4_wrap_ui(kcm_krunner_charrunner_SRCS charrunner_config.ui)
kde4_add_plugin(kcm_krunner_charrunner ${kcm_krunner_charrunner_SRCS})
target_link_libraries(kcm_krunner_charrunner
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS} 
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

# Now make sure all files get to the right place
kde4_add_plugin(krunner_charrunner ${krunner_charrunner_SRCS})
target_link_libraries(krunner_charrunner ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})
add_dependencies(krunner_charrunner kcm_krunner_charrunner)

# Install the library and .desktop file
install(TARGETS krunner_charrunner kcm_krunner_charrunner DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES CharacterRunner.desktop CharRunner_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR}) 
