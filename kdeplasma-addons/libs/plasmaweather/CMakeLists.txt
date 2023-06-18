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
    ${CMAKE_CURRENT_BINARY_DIR}/plasmaweather_export.h
)

add_library(plasmaweather SHARED ${plasmaweather_LIB_SRCS})
target_link_libraries(plasmaweather
    KDE4::plasma
    KDE4::kdeui
    KDE4Workspace::weather_ion
)

set_target_properties(plasmaweather PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(plasmaweather)

install(
    TARGETS plasmaweather
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
# nothing outside of kdeplasma-addons uses this library, and the API is not stable -> do not install headers!
# install(FILES ${plasmaweather_HEADERS} DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/plasmaweather)
