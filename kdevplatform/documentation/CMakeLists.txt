macro_log_feature(QT_QTWEBKIT_FOUND "QtWebKit" "QtWebKit, for integrated documentation" "http://qt-project.org/" FALSE)

set(kdevplatformdocumentation_LIB_SRCS
    documentationfindwidget.cpp
    documentationview.cpp
)

if(QT_QTWEBKIT_FOUND)
    set(kdevplatformdocumentation_LIB_SRCS
        ${kdevplatformdocumentation_LIB_SRCS}
        standarddocumentationview.cpp
    )
endif()

add_library(kdevplatformdocumentation SHARED ${kdevplatformdocumentation_LIB_SRCS})

install(TARGETS kdevplatformdocumentation EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

target_link_libraries(kdevplatformdocumentation
        LINK_PRIVATE ${QT_QTWEBKIT_LIBRARY}
        LINK_PUBLIC kdevplatforminterfaces)

set_target_properties(kdevplatformdocumentation PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})

install(FILES
    documentationexport.h
    documentationfindwidget.h
    standarddocumentationview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/documentation COMPONENT Devel
)