include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CTest)

check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(unistd.h HAVE_UNISTD_H)

check_function_exists(bcopy HAVE_BCOPY)
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(mmap HAVE_MMAP)

set(CMAKE_EXTRA_INCLUDE_FILES)
check_type_size(long SIZEOF_LONG)
check_type_size("long long" LONG_LONG)

configure_file(config-metakit.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-metakit.h)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/interfaces
  ${PROJECT_SOURCE_DIR}/plugins/mk4storage/metakit/include
  ${PROJECT_BINARY_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
)

set(libmetakitlocal_SRCS
  metakit/src/column.cpp
  metakit/src/custom.cpp
  metakit/src/derived.cpp
  metakit/src/field.cpp
  metakit/src/fileio.cpp
  metakit/src/format.cpp
  metakit/src/handler.cpp
  metakit/src/persist.cpp
  metakit/src/remap.cpp
  metakit/src/std.cpp
  metakit/src/store.cpp
  metakit/src/string.cpp
  metakit/src/table.cpp
  metakit/src/univ.cpp
  metakit/src/view.cpp
  metakit/src/viewx.cpp
)

########### next target ###############

set(akregator_mk4storage_plugin_PART_SRCS
  ${libmetakitlocal_SRCS}
  feedstoragemk4impl.cpp
  storagemk4impl.cpp
  storagefactorymk4impl.cpp
  mk4plugin.cpp
)

kde4_add_plugin(akregator_mk4storage_plugin ${akregator_mk4storage_plugin_PART_SRCS})

target_link_libraries(akregator_mk4storage_plugin
  ${KDE4_KDECORE_LIBS}
  syndication
  akregatorinterfaces
)

install(TARGETS akregator_mk4storage_plugin DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES akregator_mk4storage_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
