set(libkcups_SRCS
    KIppRequest.cpp
    KCupsPasswordDialog.cpp
    KCupsConnection.cpp
    KCupsJob.cpp
    KCupsPrinter.cpp
    KCupsServer.cpp
    KCupsRequest.cpp
    ClassListWidget.cpp
    PPDModel.cpp
    SelectMakeModel.cpp
    NoSelectionRectDelegate.cpp
    PrinterModel.cpp
    PrinterSortFilterModel.cpp
    JobModel.cpp
    JobSortFilterModel.cpp
    SelectMakeModel.ui
)

add_library(kcupslib SHARED ${libkcups_SRCS})
target_link_libraries(kcupslib
    ${KDE4_KIO_LIBS}
    ${CUPS_LIBRARIES}
)

install(TARGETS kcupslib ${INSTALL_TARGETS_DEFAULT_ARGS})
