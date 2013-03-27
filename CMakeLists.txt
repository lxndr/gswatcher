cmake_minimum_required (VERSION 2.8)
project (gswatcher C)

set (GS_VERSION "1.5.1")
set (GS_HOMEPAGE "http://gswatcher.sourceforge.net")
set (GS_LANGUAGES ru)

include (GNUInstallDirs)
find_package (PkgConfig)

pkg_check_modules (GLIB REQUIRED glib-2.0>=2.32 gobject-2.0>=2.32
		gio-2.0>=2.32 gmodule-2.0>=2.32)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0>=3.4)

if (WIN32)
	set (GEOIP_LIBRARIES GeoIP ws2_32)
else ()
	set (GEOIP_LIBRARIES GeoIP)
endif ()

set (GS_LIBRARIES m)


option (GSWATCHER_BUILD_TESTS "Build tests" ON)
option (GSWATCHER_ENABLE_NLS "National language support" ON)
option (GSWATCHER_ABS_PATHS "Use absolute paths" ON)


set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -DDEBUG")

set (CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -pg")
set (CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")


add_subdirectory (src)
if (GSWATCHER_ENABLE_NLS)
	add_subdirectory (po)
endif ()
if (GSWATCHER_BUILD_TESTS)
	add_subdirectory (tests)
endif ()


install (FILES icons/gswatcher.png
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/pixmaps)
install (DIRECTORY icons/flags
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gswatcher/icons
			FILES_MATCHING PATTERN "*.png")
install (DIRECTORY icons/games
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gswatcher/icons
			FILES_MATCHING PATTERN "*.png")
install (DIRECTORY sounds
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gswatcher)
install (FILES gswatcher.desktop
			DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/applications)
install (FILES LICENSE README
			DESTINATION ${CMAKE_INSTALL_FULL_DOCDIR})
