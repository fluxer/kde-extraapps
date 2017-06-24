include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckStructMember)
include(MacroBoolTo01)

macro_bool_to_01(OGGVORBIS_FOUND HAVE_VORBIS)

check_include_files(machine/endian.h HAVE_MACHINE_ENDIAN_H)
# Linux has <endian.h>, FreeBSD has <sys/endian.h> and Solaris has neither.
check_include_files(endian.h HAVE_ENDIAN_H)
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
check_include_files(unistd.h HAVE_UNISTD_H)
