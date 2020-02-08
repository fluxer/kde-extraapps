project(plasma-weatherstation)

set(weatherstation_SRCS
    weatherstation.cpp
    lcd.cpp
    appearanceconfig.ui
)

kde4_add_plugin(plasma_applet_weatherstation ${weatherstation_SRCS})
target_link_libraries(plasma_applet_weatherstation
    ${KDE4_KUNITCONVERSION_LIBS}
    plasmaweather
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4WORKSPACE_WEATHERION_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY})

install(TARGETS plasma_applet_weatherstation DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-weatherstation.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES
    lcd2.svgz
    lcd_panel.svgz
    lcd_digits.svgz
    wind_arrows.svgz
    weather_icons.svgz
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/weatherstation/)

install(DIRECTORY package/ DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma/packages/org.kde.lcdweather)
