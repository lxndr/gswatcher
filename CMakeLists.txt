cmake_minimum_required (VERSION 2.8)
project (gstool C)

set (GS_VERSION "0.5")

find_package (PkgConfig)

add_subdirectory (src)

install (FILES icons/gstool.png
			DESTINATION share/pixmaps)

install (DIRECTORY icons/flags
			DESTINATION share/gstool/icons
			FILES_MATCHING PATTERN "*.png")
install (DIRECTORY icons/games
			DESTINATION share/gstool/icons
			FILES_MATCHING PATTERN "*.png")
install (DIRECTORY sounds
			DESTINATION share/gstool)
install (FILES gstool.desktop
			DESTINATION share/applications)
