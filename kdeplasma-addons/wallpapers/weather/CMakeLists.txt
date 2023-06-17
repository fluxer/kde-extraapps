project(plasma-wallpaper-weather)

include_directories(
    # for plasmaweather_export.h
    ${kdeplasma-addons_BINARY_DIR}/libs/plasmaweather
)

set(weather_SRCS
    weatherwallpaper.cpp
    backgrounddelegate.cpp
    backgroundlistmodel.cpp
)

kde4_add_plugin(plasma_wallpaper_weather ${weather_SRCS})
target_link_libraries(plasma_wallpaper_weather KDE4::plasma KDE4::kio KDE4::kfile plasmaweather)

install(TARGETS plasma_wallpaper_weather DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-weather.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
