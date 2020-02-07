add_subdirectory(plasmaweather)
add_subdirectory(lancelot)

if(KDE4WORKSPACE_FOUND)
   add_subdirectory(lancelot-datamodels)
endif(KDE4WORKSPACE_FOUND)
