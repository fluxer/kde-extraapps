project(libbluedevil)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "${KDE4_BIN_INSTALL_DIR}"
                                  LIBRARY DESTINATION "${KDE4_LIB_INSTALL_DIR}"
                                  ARCHIVE DESTINATION "${KDE4_LIB_INSTALL_DIR}" COMPONENT Devel )


set(GENERIC_LIB_VERSION "2.1")
set(GENERIC_LIB_SOVERSION "2")
set(VERSION ${GENERIC_LIB_VERSION})

add_subdirectory(bluedevil)

option(LIBBLUEDEVIL_BUILD_API_DOCS "Build libbluedevil API documentation")

##################  apidox ################################
if(LIBBLUEDEVIL_BUILD_API_DOCS)
  find_package(Doxygen)

  if(DOXYGEN_EXECUTABLE)
    if(EXISTS "${QT_DOC_DIR}/html/qt.tags")
      set(QT_TAG_STRING "${QT_DOC_DIR}/html/qt.tags=${QT_DOC_DIR}/html")
    endif()

    configure_file(${libbluedevil_SOURCE_DIR}/.Doxyfile.cmake ${libbluedevil_BINARY_DIR}/Doxyfile)

    add_custom_target(
      apidox ALL
      COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    )
  endif(DOXYGEN_EXECUTABLE)
endif(LIBBLUEDEVIL_BUILD_API_DOCS)
