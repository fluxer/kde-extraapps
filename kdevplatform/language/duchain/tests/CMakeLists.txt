add_subdirectory(serialization)

########### next target ###############

set(duchaintest_SRCS test_duchain.cpp)

kde4_add_test(kdevplatform-duchaintest ${duchaintest_SRCS})
target_link_libraries(kdevplatform-duchaintest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)

########### next target ###############

set(itemrepositorytest_SRCS test_itemrepository.cpp)

kde4_add_test(kdevplatform-itemrepositorytest ${itemrepositorytest_SRCS})
target_link_libraries(kdevplatform-itemrepositorytest
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    kdevplatformlanguage
    ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_automatic_test_SRCS test_itemrepositoryregistry_automatic.cpp)
kde4_add_test(kdevplatform-itemrepositoryregistry_automatic_test ${itemrepositoryregistry_automatic_test_SRCS})
target_link_libraries(kdevplatform-itemrepositoryregistry_automatic_test ${QT_QTTEST_LIBRARY}
    kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(itemrepositoryregistry_deferred_test_SRCS test_itemrepositoryregistry_deferred.cpp)
kde4_add_test(kdevplatform-itemrepositoryregistry_deferred_test ${itemrepositoryregistry_deferred_test_SRCS})
target_link_libraries(kdevplatform-itemrepositoryregistry_deferred_test ${QT_QTTEST_LIBRARY}
    kdevplatformlanguage ${KDEVPLATFORM_TESTS_LIBRARIES})

########### next target ###############

set(benchhashes_SRCS benchhashes.cpp)

kde4_add_test(kdevplatform-benchhashes ${benchhashes_SRCS})
target_link_libraries(kdevplatform-benchhashes
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
)
