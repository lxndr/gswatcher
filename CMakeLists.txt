cmake_minimum_required (VERSION 2.8)
project (gstool C)

set (GS_VERSION "1.0")
set (GS_HOMEPAGE "http://gstool.sourceforge.net")
set (GS_LANGUAGES ru)

include (GNUInstallDirs)
find_package (PkgConfig)

pkg_check_modules (GLIB REQUIRED glib-2.0>=2.30 gobject-2.0>=2.30
		gio-2.0>=2.30 gmodule-2.0>=2.30)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0>=3.2)
if (UNIX AND NOT APPLE)
	pkg_check_modules (CANBERRA REQUIRED libcanberra)
endif ()


option (GSTOOL_BUILD_TESTS "Build tests" ON)
option (GSTOOL_ENABLE_NLS "National language support" ON)
option (GSTOOL_ABS_PATHS "Use absolute paths" ON)


set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -DDEBUG")

set (CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -pg")
set (CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")


add_subdirectory (src)
if (GSTOOL_ENABLE_NLS)
	add_subdirectory (po)
endif ()
if (GSTOOL_BUILD_TESTS)
	add_subdirectory (tests)
endif ()


install (FILES icons/gstool.png
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/pixmaps)
install (DIRECTORY icons/flags
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gstool/icons
			FILES_MATCHING PATTERN "*.png")
install (DIRECTORY icons/games
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gstool/icons
			FILES_MATCHING PATTERN "*.png")
install (DIRECTORY sounds
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gstool)
install (FILES gstool.desktop
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications)
install (FILES LICENSE README
			DESTINATION ${CMAKE_INSTALL_FULL_DOCDIR})


