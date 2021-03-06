project(LIBKTORRENT)

find_package(GMP)
set_package_properties(GMP PROPERTIES
    DESCRIPTION "The GNU Multiple Precision Arithmetic Library"
    URL "http://gmplib.org/"
    PURPOSE "Required for building libktorrent"
    TYPE REQUIRED
)

find_package(LibGcrypt)
set_package_properties(LibGcrypt PROPERTIES
    DESCRIPTION "General purpose cryptographic library based on the code from GnuPG"
    URL "https://gnupg.org/gcrypt/"
    PURPOSE "Required for building libktorrent"
    TYPE REQUIRED
)

# make some more macros available
include (MacroLibrary)

#
# checks for functions and stuff
#
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize) #XFS

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
include_directories(${LIBKTORRENT_BINARY_DIR} ${LIBKTORRENT_SOURCE_DIR} ${GMP_INCLUDE_DIR})

set(LIBKTORRENT_VERSION "1.1dev")

#XFS prealloc
CHECK_INCLUDE_FILES(xfs/xfs.h HAVE_XFS_XFS_H)
IF(HAVE_XFS_XFS_H)
	SET(CMAKE_EXTRA_INCLUDE_FILES xfs/xfs.h)
	CHECK_TYPE_SIZE(__u64 HAVE___U64)
	CHECK_TYPE_SIZE(__s64 HAVE___S64)
	SET(CMAKE_EXTRA_INCLUDE_FILES)
ENDIF(HAVE_XFS_XFS_H)

# check for 64 bit file I/O functions
check_function_exists(fopen64 HAVE_FOPEN64)
check_function_exists(fseeko64 HAVE_FSEEKO64)
check_function_exists(fseeko HAVE_FSEEKO)
check_function_exists(ftello64 HAVE_FTELLO64)
check_function_exists(ftello HAVE_FTELLO)
check_function_exists(fstat64 HAVE_FSTAT64)
check_function_exists(stat64 HAVE_STAT64)
check_function_exists(ftruncate64 HAVE_FTRUNCATE64)
check_function_exists(lseek64 HAVE_LSEEK64)
check_function_exists(mmap64 HAVE_MMAP64)
check_function_exists(munmap64 HAVE_MUNMAP64)
check_function_exists(posix_fallocate64 HAVE_POSIX_FALLOCATE64)
check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
check_function_exists(fallocate HAVE_FALLOCATE)
check_function_exists(statvfs HAVE_STATVFS)
check_function_exists(statvfs64 HAVE_STATVFS64)


add_subdirectory(src)
add_subdirectory(testlib)
add_subdirectory(examples)
add_subdirectory(cmake)

