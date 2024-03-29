set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(JSONINTERFACE_SOURCES
    jsonarchiveinterface.cpp
    jsonparser.cpp
)
add_library(jsoninterface STATIC ${JSONINTERFACE_SOURCES})

macro(KERFUFFLE_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kde4_add_test(kerfuffle-${_testname} ${_testname}.cpp)
        target_link_libraries(kerfuffle-${_testname}
            jsoninterface
            kerfuffle
            KDE4::kdeui
            ${QT_QTTEST_LIBRARY}
        )
    endforeach(_testname)
endmacro(KERFUFFLE_UNIT_TESTS)

KERFUFFLE_UNIT_TESTS(
    archivetest
    jobstest
)
