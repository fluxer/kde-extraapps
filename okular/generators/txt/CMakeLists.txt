include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_BINARY_DIR}/okular
)

########### next target ###############

set(okularGenerator_txt_SRCS
   generator_txt.cpp
   converter.cpp
   document.cpp
)


kde4_add_plugin(okularGenerator_txt ${okularGenerator_txt_SRCS})

target_link_libraries(okularGenerator_txt okularcore KDE4::kdecore KDE4::kdeui ${QT_QTGUI_LIBRARY})

install(TARGETS okularGenerator_txt DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_txt.desktop okularTxt.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_txt.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )
