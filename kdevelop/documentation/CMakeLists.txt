macro_log_feature(QT_QTHELP_FOUND "QtHelp" "The Help module for the Qt toolkit, needed for the qthelp plugin" "http://qt-project.org/" FALSE)
macro_log_feature(QT_QTWEBKIT_FOUND "QtWebKit" "The WebKit module for the Qt toolkit, needed for the qthelp and manpage plugin" "http://qt-project.org/" FALSE)

# TODO: that should use a part
if(QT_QTHELP_FOUND AND QT_QTWEBKIT_FOUND)
 macro_optional_add_subdirectory(qthelp)
    macro_optional_add_subdirectory(manpage)
endif()
