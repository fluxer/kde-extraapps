include_directories(
    ${gwenview_SOURCE_DIR}
    )
# SlideContainer
set(slidecontainertest_SRCS
    slidecontainertest.cpp
    )

kde4_add_manual_test(slidecontainertest ${slidecontainertest_SRCS})

target_link_libraries(slidecontainertest ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} gwenviewlib)

# imageloadbench
set(imageloadbench_SRCS
    imageloadbench.cpp
    )

kde4_add_manual_test(imageloadbench ${imageloadbench_SRCS})

target_link_libraries(imageloadbench ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} gwenviewlib)
