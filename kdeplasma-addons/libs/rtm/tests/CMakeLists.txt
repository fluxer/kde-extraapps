include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

if(QT_QTWEBKIT_FOUND)
    add_executable(rtmlogin login.cpp)
    target_link_libraries(rtmlogin
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDEWEBKIT_LIBS}
        rtm
        ${QT_QTSCRIPT_LIBRARY}
    )
endif()

#add_executable(rtmrefresh refresh.cpp)
#target_link_libraries(rtmrefresh ${KDE4_KDEUI_LIBS} rtm ${QT_QTSCRIPT_LIBRARY})
