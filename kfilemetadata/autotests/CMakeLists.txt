configure_file(
    indexerextractortestsconfig.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/indexerextractortestsconfig.h @ONLY
)

kde4_add_test(kfilemetadata-extractortests
    indexerextractortests.cpp
    simpleresult.cpp
    ../src/extractors/plaintextextractor.cpp
)

target_link_libraries(kfilemetadata-extractortests
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kfilemetadata
)

#
# Property Info
#
kde4_add_test(kfilemetadata-propertyinfotest
    propertyinfotest.cpp
)

target_link_libraries(kfilemetadata-propertyinfotest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kfilemetadata
)

