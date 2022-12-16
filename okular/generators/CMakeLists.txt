# let's enable the generators properly configured

if(Poppler_FOUND)
    add_subdirectory(poppler)
endif()

if(LibSpectre_FOUND)
    add_subdirectory(spectre)
endif()

if(DjVuLibre_FOUND)
    add_subdirectory(djvu)
endif()

if(EPub_FOUND)
    add_subdirectory(epub)
endif()

add_subdirectory(comicbook)
add_subdirectory(fictionbook)
add_subdirectory(markdown)
add_subdirectory(txt)
