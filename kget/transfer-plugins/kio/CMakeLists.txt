include_directories(
   ../../
    ${CMAKE_CURRENT_BINARY_DIR}/../../
)

set(kget_kiofactory_PART_SRCS
  transferKio.cpp
  transferKioFactory.cpp
)

kde4_add_plugin(kget_kiofactory ${kget_kiofactory_PART_SRCS})

target_link_libraries(kget_kiofactory KDE4::kio kgetcore)

install(TARGETS kget_kiofactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kget_kiofactory.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
