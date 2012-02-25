pkg_check_modules (GTK gtk+-3.0)
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

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -DDEBUG")

set (CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_DEBUG} -pg")
set (CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")

include_directories (${GTK_INCLUDE_DIRS})

add_executable (test-querier
		../src/query/querier.c
		../src/query/proto-source.c
		../src/query/updater.c
		../src/query/utils.c
		test-querier.c)
target_link_libraries (test-querier
		${GTK_LIBRARIES}
		${GS_LIBRARIES})

add_executable (test-watcher
		../src/query/querier.c
		../src/query/watcher.c
		../src/query/proto-source.c
		../src/query/updater.c
		../src/query/utils.c
		test-watcher.c)
target_link_libraries (test-watcher
		${GTK_LIBRARIES}
		${GS_LIBRARIES})
