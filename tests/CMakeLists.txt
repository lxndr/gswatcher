set (GS_LIBRARIES m)

include_directories (${GLIB_INCLUDE_DIRS})

add_executable (test-querier
		../src/query/querier.c
		../src/query/proto-gamespy.c
		../src/query/proto-gamespy3.c
		../src/query/proto-quake3.c
		../src/query/proto-source.c
		../src/query/updater.c
		../src/query/utils.c
		test-querier.c)
target_link_libraries (test-querier
		${GLIB_LIBRARIES}
		${GS_LIBRARIES})
