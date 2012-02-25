pkg_check_modules (GTK3 REQUIRED gtk+-3.0>=3.2)
if (UNIX AND NOT APPLE)
	pkg_check_modules (CANBERRA REQUIRED libcanberra)
endif ()

if (WIN32)
	set (GEOIP_LIBRARIES GeoIP ws2_32)
	set (GS_LIBRARIES winmm m)
else ()
	set (GEOIP_LIBRARIES GeoIP)
	set (GS_LIBRARIES m)
endif ()

include_directories (${GTK3_INCLUDE_DIRS} ${CANBERRA_INCLUDE_DIRS})
add_definitions (
		-DGETTEXT_PACKAGE=\"gstool\"
		-DGS_VERSION=\"${GS_VERSION}\")
if (CMAKE_INSTALL_DATADIR)
	add_definitions (-DGS_DATADIR=\"${CMAKE_INSTALL_DATADIR}\")
endif ()
if (CMAKE_INSTALL_LOCALEDIR)
	add_definitions (-DENABLE_NLS -DGS_LOCALEDIR=\"${CMAKE_INSTALL_LOCALEDIR}\")
endif ()

add_executable (gstool
		query/querier.c
		query/console.c
		query/proto-source.c
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
		${GTK3_LIBRARIES}
		${CANBERRA_LIBRARIES}
		${GEOIP_LIBRARIES}
		${GS_LIBRARIES})

install (TARGETS gstool DESTINATION bin)
