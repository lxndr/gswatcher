cmake_minimum_required (VERSION 2.8)
project (gstool C)

set (GS_VERSION "1.0-multiprotocol")
set (GS_LANGUAGES ru)

find_package (PkgConfig)


option (GSTOOL_BUILD_TESTS "Build tests" OFF)


set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -DDEBUG")

set (CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -pg")
set (CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")


add_subdirectory (src)
add_subdirectory (po)
if (GSTOOL_BUILD_TESTS)
	add_subdirectory (tests)
endif ()


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
