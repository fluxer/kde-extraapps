project(context-labels)

set(labels_SRCS
    LabelsApplet.cpp 
    LabelGraphicsItem.cpp
    LabelOverlayButton.cpp
    labelsGeneralSettings.ui
    labelsBlacklistSettings.ui
    labelsReplacementSettings.ui
)

include_directories( ../..
                     ../../.. )

kde4_add_plugin(amarok_context_applet_labels ${labels_SRCS})
target_link_libraries(amarok_context_applet_labels
    amarokcore
    amaroklib
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS amarok_context_applet_labels DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES amarok-context-applet-labels.desktop DESTINATION ${SERVICES_INSTALL_DIR})
