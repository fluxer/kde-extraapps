# pics/ contains non-themed icons and other graphical resources

# We don't find them yet externally, so disable installation
set(CLIENT_RCS ${CLIENT_RCS} ${CMAKE_CURRENT_SOURCE_DIR}/pics.qrc PARENT_SCOPE)
# install(FILES qt-logo.png
#               quassel-large.png
#         DESTINATION ${DATA_INSTALL_DIR}/quassel/pics)
