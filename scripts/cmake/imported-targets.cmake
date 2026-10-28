# Include instead of find_package(Qt6) for a Linux-hosted MinGW cross build.
# The application uses no custom Q_OBJECT classes or qrc, so no host Qt tools are needed.
if(NOT EMCICALC_MINGW_PREFIX)
  message(FATAL_ERROR "Set EMCICALC_MINGW_PREFIX to the Windows dependency prefix")
endif()
foreach(module Core Gui Widgets Network)
  add_library(Qt6::${module} SHARED IMPORTED GLOBAL)
  string(TOUPPER "${module}" module_upper)
  set_target_properties(Qt6::${module} PROPERTIES
    IMPORTED_IMPLIB "${EMCICALC_MINGW_PREFIX}/lib/libQt6${module}.dll.a"
    IMPORTED_LOCATION "${EMCICALC_MINGW_PREFIX}/bin/Qt6${module}.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${EMCICALC_MINGW_PREFIX}/include/qt6;${EMCICALC_MINGW_PREFIX}/include/qt6/Qt${module}"
    INTERFACE_COMPILE_DEFINITIONS "QT_${module_upper}_LIB;QT_NO_DEBUG")
endforeach()
set_target_properties(Qt6::Network PROPERTIES INTERFACE_LINK_LIBRARIES "Qt6::Core")
set_target_properties(Qt6::Gui PROPERTIES INTERFACE_LINK_LIBRARIES "Qt6::Core")
set_target_properties(Qt6::Widgets PROPERTIES INTERFACE_LINK_LIBRARIES "Qt6::Gui;Qt6::Core")
add_library(Qalculate::Qalculate SHARED IMPORTED GLOBAL)
set_target_properties(Qalculate::Qalculate PROPERTIES
  IMPORTED_NO_SYSTEM TRUE
  IMPORTED_IMPLIB "${EMCICALC_MINGW_PREFIX}/lib/libqalculate.dll.a"
  IMPORTED_LOCATION "${EMCICALC_MINGW_PREFIX}/bin/libqalculate-23.dll"
  INTERFACE_INCLUDE_DIRECTORIES "${EMCICALC_MINGW_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "${EMCICALC_MINGW_PREFIX}/lib/libmpfr.dll.a;${EMCICALC_MINGW_PREFIX}/lib/libgmp.dll.a")
