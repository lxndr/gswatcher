pkg_check_modules (GLIB REQUIRED glib-2.0>=2.30 gobject-2.0>=2.30 gio-2.0>=2.30)

if (WIN32)
	set (GS_LIBRARIES m)
else ()
	set (GS_LIBRARIES m)
endif ()

include_directories (${GLIB_INCLUDE_DIRS})

add_executable (test-querier
		../src/query/querier.c
		../src/query/proto-gamespy.c
		../src/query/proto-quake3.c
		../src/query/proto-source.c
		../src/query/updater.c
		../src/query/utils.c
		test-querier.c)
target_link_libraries (test-querier
		${GLIB_LIBRARIES}
		${GS_LIBRARIES})

add_executable (test-watcher
		../src/query/querier.c
		../src/query/watcher.c
		../src/query/proto-gamespy.c
		../src/query/proto-quake3.c
		../src/query/proto-source.c
		../src/query/updater.c
		../src/query/utils.c
		test-watcher.c)
target_link_libraries (test-watcher
		${GLIB_LIBRARIES}
		${GS_LIBRARIES})
