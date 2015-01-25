# Builds the common module

set(SOURCES
    aliasmanager.cpp
    authhandler.cpp
    backlogmanager.cpp
    basichandler.cpp
    bufferinfo.cpp
    buffersyncer.cpp
    bufferviewconfig.cpp
    bufferviewmanager.cpp
    cliparser.cpp
    compressor.cpp
    ctcpevent.cpp
    event.cpp
    eventmanager.cpp
    identity.cpp
    ignorelistmanager.cpp
    internalpeer.cpp
    ircchannel.cpp
    ircevent.cpp
    irclisthelper.cpp
    ircuser.cpp
    logger.cpp
    message.cpp
    messageevent.cpp
    network.cpp
    networkconfig.cpp
    networkevent.cpp
    peer.cpp
    peerfactory.cpp
    presetnetworks.cpp
    quassel.cpp
    remotepeer.cpp
    settings.cpp
    signalproxy.cpp
    syncableobject.cpp
    transfer.cpp
    transfermanager.cpp
    util.cpp

    protocols/datastream/datastreampeer.cpp
    protocols/legacy/legacypeer.cpp

    # needed for automoc
    coreinfo.h
)


if (QCA2_FOUND)
    set(SOURCES ${SOURCES} keyevent.cpp)
endif()

include_directories(${ZLIB_INCLUDE_DIRS})

if (HAVE_SYSLOG)
    add_definitions(-DHAVE_SYSLOG)
endif()

if (EXECINFO_FOUND)
    add_definitions(-DHAVE_EXECINFO)
    include_directories(${EXECINFO_INCLUDES})
endif()
set(SOURCES ${SOURCES} logbacktrace_unix.cpp)

qt4_add_resources(SOURCES ${COMMON_RCS})

add_library(mod_common STATIC ${SOURCES})

target_link_libraries(mod_common
    ${CMAKE_DL_LIBS}
    ${EXECINFO_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
)

