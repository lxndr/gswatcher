if (APPLE)
	add_definitions (-DG_OS_MACOSX)
endif ()

include_directories (${GLIB_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})

add_definitions (
		-DGETTEXT_PACKAGE=\"gswatcher\"
		-DGS_VERSION=\"${GS_VERSION}\"
		-DGS_HOMEPAGE=\"${GS_HOMEPAGE}\")

if (GSWATCHER_ABS_PATHS)
	add_definitions (-DGS_DATADIR=\"${CMAKE_INSTALL_FULL_DATAROOTDIR}\")
endif ()

if (GSWATCHER_ENABLE_NLS)
	add_definitions (-DENABLE_NLS)
	if (GSWATCHER_ABS_PATHS)
		add_definitions (-DGS_LOCALEDIR=\"${CMAKE_INSTALL_FULL_LOCALEDIR}\")
	endif ()
endif ()

add_executable (gswatcher
		gtkpopupbutton.c
		query/querier.c
		query/watcher.c
		query/proto-gamespy.c
		query/proto-gamespy3.c
		query/proto-quake.c
		query/proto-source.c
		query/console.c
		query/console-source.c
		query/console-telnet.c
		query/utils.c
		platform.c
		client.c
		json.c
		gswatcher.c
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

target_link_libraries (gswatcher
		${GLIB_LIBRARIES}
		${GTK3_LIBRARIES}
		${GEOIP_LIBRARIES}
		${GS_LIBRARIES})

install (TARGETS gswatcher
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
