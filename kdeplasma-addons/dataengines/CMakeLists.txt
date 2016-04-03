if (QCA2_FOUND AND QJSON_FOUND AND QTOAUTH_FOUND)
      add_subdirectory(microblog)
endif()

if(LIBATTICA_FOUND)
  add_subdirectory(ocs)
endif()

add_subdirectory(potd)
add_subdirectory(rememberthemilk)
add_subdirectory(kdeobservatory)
add_subdirectory(kimpanel)
add_subdirectory(konsoleprofiles)
add_subdirectory(konqprofiles)
