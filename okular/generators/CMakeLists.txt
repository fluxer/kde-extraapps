# let's enable the generators properly configured

if(PopplerQt4_FOUND)
    add_subdirectory(poppler)
endif()

if(LibSpectre_FOUND)
    add_subdirectory(spectre)
endif()

if(DjVuLibre_FOUND)
    add_subdirectory(djvu)
endif()

add_subdirectory(dvi)

add_subdirectory(xps)

add_subdirectory(ooo)

add_subdirectory(fictionbook)

add_subdirectory(comicbook)

add_subdirectory(fax)

if(JPEG_FOUND AND ZLIB_FOUND)
    add_subdirectory(plucker)
endif()

if(EPub_FOUND)
    add_subdirectory(epub)
endif()

add_subdirectory(markdown)
add_subdirectory(txt)
