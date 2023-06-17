include_directories(
   ${CMAKE_BINARY_DIR}/okular
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${POPPLER_INCLUDE_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=4710)

########### next target ###############

set(okularGenerator_poppler_PART_SRCS
    generator_pdf.cpp
)

kde4_add_plugin(okularGenerator_poppler ${okularGenerator_poppler_PART_SRCS})

target_link_libraries(okularGenerator_poppler
    KDE4::kdeui
    ${POPPLER_LIBRARIES}
    okularcore
)

install(
    TARGETS okularGenerator_poppler
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES libokularGenerator_poppler.desktop okularPoppler.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

install(
    PROGRAMS okularApplication_pdf.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

