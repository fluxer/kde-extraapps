macro(gv_add_unit_test _test)
    kde4_add_test(${_test} ${_test}.cpp ${ARGN})
    target_link_libraries(${_test}
        ${KDE4_KIO_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDCRAW_LIBS}
        ${QT_QTTEST_LIBRARY}
        ${QT_QTCORE_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        gwenviewlib
    )
endmacro(gv_add_unit_test)

include_directories(
    ${gwenview_SOURCE_DIR}
    ${importer_SOURCE_DIR}
)

# For config-gwenview.h
include_directories(${gwenview_BINARY_DIR})

gv_add_unit_test(imagescalertest testutils.cpp)
gv_add_unit_test(paintutilstest)
gv_add_unit_test(documenttest testutils.cpp)
gv_add_unit_test(transformimageoperationtest)
gv_add_unit_test(timeutilstest)
gv_add_unit_test(placetreemodeltest testutils.cpp)
gv_add_unit_test(urlutilstest)
gv_add_unit_test(historymodeltest)
gv_add_unit_test(importertest
    ${importer_SOURCE_DIR}/importer.cpp
    ${importer_SOURCE_DIR}/fileutils.cpp
    ${importer_SOURCE_DIR}/filenameformater.cpp
)
gv_add_unit_test(sorteddirmodeltest testutils.cpp)
gv_add_unit_test(slidecontainerautotest slidecontainerautotest.cpp)
gv_add_unit_test(imagemetainfomodeltest testutils.cpp)
gv_add_unit_test(recursivedirmodeltest testutils.cpp)
gv_add_unit_test(contextmanagertest testutils.cpp)
