include_directories( ${CMAKE_SOURCE_DIR}/knumber  )

set(knumbertest_SRCS knumbertest.cpp ${libknumber_la_SRCS})

kde4_add_test(knumber-KNumber ${knumbertest_SRCS})

target_link_libraries(knumber-KNumber KDE4::kdecore ${GMP_LIBRARIES})
