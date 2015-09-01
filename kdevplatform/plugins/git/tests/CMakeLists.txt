# Running the test only makes sense if the git command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(GIT_FOUND NAMES git)

if (GIT_FOUND)

    set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
    set(gittest_SRCS
        gittest.cpp
        ../gitplugin.cpp
        ../gitclonejob.cpp
        ../stashmanagerdialog.cpp
        ../stashpatchsource.cpp
        ../gitjob.cpp
        ../gitmessagehighlighter.cpp
        ../gitplugincheckinrepositoryjob.cpp
        ./stashmanagerdialog.ui
    )
    kde4_add_test(kdevgit-test ${gittest_SRCS})
    target_link_libraries(kdevgit-test
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VCS_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
    )
endif (GIT_FOUND)
