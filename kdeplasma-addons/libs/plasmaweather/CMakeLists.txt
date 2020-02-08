set(plasmaweather_LIB_SRCS
    weatherpopupapplet.cpp
    weathervalidator.cpp
    weatherlocation.cpp
    weatherconfig.cpp
    weatheri18ncatalog.cpp
    weatherconfig.ui
)

set(plasmaweather_HEADERS
    weatherpopupapplet.h
    weathervalidator.h
    weatherconfig.h
    weatherlocation.h
    plasmaweather_export.h
)

add_library(plasmaweather SHARED ${plasmaweather_LIB_SRCS})

target_link_libraries(plasmaweather ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KUNITCONVERSION_LIBS} ${KDE4WORKSPACE_WEATHERION_LIBRARY})
set_target_properties(plasmaweather PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS plasmaweather ${INSTALL_TARGETS_DEFAULT_ARGS})
# nothing outside of kdeplasma-addons uses this library, and the API is not stable -> do not install headers!
#install(FILES ${plasmaweather_HEADERS} DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/plasmaweather COMPONENT Devel)

install(FILES plasmaweather.knsrc DESTINATION ${KDE4_CONFIG_INSTALL_DIR})
