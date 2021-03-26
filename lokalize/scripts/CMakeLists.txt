
install(
    FILES
    newprojectwizard.rc
    newprojectwizard.py
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/lokalize/scripts
)

install(
    PROGRAMS find-gui-text.sh
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/lokalize/scripts
)

install(
    FILES
    msgmerge.py
    msgmerge.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/lokalize/scripts
)

install(
    FILES
    xliff2odf.py
    xliff2odf.rc
    xliff2odf-standalone.py
    xliffmerge.py
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/lokalize/scripts/odf
)
