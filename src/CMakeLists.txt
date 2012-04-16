if (APPLE)
	add_definitions (-DG_OS_MACOSX)
endif ()

if (WIN32)
	set (GEOIP_LIBRARIES GeoIP ws2_32)
	set (GS_LIBRARIES winmm m)
else ()
	set (GEOIP_LIBRARIES GeoIP)
	set (GS_LIBRARIES m)
endif ()

include_directories (${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS}
		${CANBERRA_INCLUDE_DIRS})

add_definitions (
		-DGETTEXT_PACKAGE=\"gstool\"
		-DGS_VERSION=\"${GS_VERSION}\"
		-DGS_HOMEPAGE=\"${GS_HOMEPAGE}\")

if (GSTOOL_ABS_PATHS)
	add_definitions (-DGS_DATADIR=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}\")
endif ()

if (GSTOOL_ENABLE_NLS)
	add_definitions (-DENABLE_NLS)
	if (GSTOOL_ABS_PATHS)
		add_definitions (-DGS_LOCALEDIR=\"${CMAKE_INSTALL_FULL_LOCALEDIR}\")
	endif ()
endif ()

add_executable (gstool
		query/querier.c
		query/proto-gamespy.c
		query/proto-gamespy3.c
		query/proto-quake3.c
		query/proto-source.c
		query/console.c
		query/utils.c
		platform.c
		client.c
		json.c
		gstool.c
		gui-buddy-list.c
		gui-console.c
		gui-info.c
		gui-player-list.c
		gui-server-list.c
		gui-preferences.c
		gui-window.c
		gui-notification.c
		gui-log.c
		gui-chat.c)

target_link_libraries (gstool
		${GLIB_LIBRARIES}
		${GTK3_LIBRARIES}
		${CANBERRA_LIBRARIES}
		${GEOIP_LIBRARIES}
		${GS_LIBRARIES})

install (TARGETS gstool
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
