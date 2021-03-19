macro_optional_find_package(PopplerQt4 0.28.0)
set_package_properties(PopplerQt4 PROPERTIES
    DESCRIPTION "A PDF rendering library"
    URL "http://poppler.freedesktop.org"
    PURPOSE "Support for PDF files in Okular"
)

macro_optional_find_package(LibSpectre 0.2)
set_package_properties(LibSpectre PROPERTIES
    DESCRIPTION "A PostScript rendering library"
    URL "http://libspectre.freedesktop.org/wiki/"
    PURPOSE "Support for PS files in Okular"
)

macro_optional_find_package(DjVuLibre 3.5.17)
set_package_properties(DjVuLibre PROPERTIES
    DESCRIPTION "A library for dealing with DjVu formatted files"
    URL "http://djvulibre.djvuzone.org"
    PURPOSE "Support for DjVu files in Okular"
)

macro_optional_find_package(TIFF)
set_package_properties(TIFF PROPERTIES
    DESCRIPTION "A library for reading and writing TIFF formatted files"
    URL "http://www.remotesensing.org/libtiff"
    PURPOSE "Support for TIFF files in Okular"
)

macro_optional_find_package(Freetype)
set_package_properties(Freetype PROPERTIES
    DESCRIPTION "A font rendering engine"
    URL "http://www.freetype.org"
    PURPOSE "Provides freetype font support in the Okular DVI generator"
)

macro_optional_find_package(JPEG)
set_package_properties(JPEG PROPERTIES
    DESCRIPTION "A library for reading and writing JPEG image files"
    URL "http://www.ijg.org"
    PURPOSE "Support fof PalmDB documents in Okular"
)

macro_optional_find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "General purpose data compression library"
    URL "http://www.zlib.net"
    PURPOSE "Support for Plucker files in Okular"
)

macro_optional_find_package(EPub)
set_package_properties(EPub PROPERTIES
    DESCRIPTION "A library for reading EPub documents"
    URL "http://sourceforge.net/projects/ebook-tools"
    PURPOSE "Support for EPub documents in Okular"
)

macro_optional_find_package(QMobipocket)
set_package_properties(QMobipocket PROPERTIES
    DESCRIPTION "A library for reading Mobipocket documents"
    URL "https://projects.kde.org/projects/kde/kdegraphics/kdegraphics-mobipocket"
    PURPOSE "Support for Mobipocket documents in Okular"
)

macro_optional_find_package(LibGcrypt)
set_package_properties(LibGcrypt PROPERTIES
    DESCRIPTION "General purpose cryptographic library based on the code from GnuPG"
    URL "https://gnupg.org/gcrypt/"
    PURPOSE "Support encrypted Open Document in Okular"
    TYPE REQUIRED
)

# let's enable the generators properly configured

if(PopplerQt4_FOUND)
    add_subdirectory(poppler)
endif()

if(LibSpectre_FOUND)
    add_subdirectory(spectre)
endif()

add_subdirectory(kimgio)

if(DjVuLibre_FOUND)
    add_subdirectory(djvu)
endif()

add_subdirectory(dvi)

if(TIFF_FOUND)
    add_subdirectory(tiff)
endif()

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

add_subdirectory(txt)

if(QMobipocket_FOUND)
    add_subdirectory(mobipocket)
endif()
