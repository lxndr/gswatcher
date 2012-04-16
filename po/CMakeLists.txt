find_program (XGETTEXT_EXECUTABLE xgettext)
find_program (MSGMERGE_EXECUTABLE msgmerge)
find_program (MSGFMT_EXECUTABLE msgfmt)

set (POT_FILE ${CMAKE_PROJECT_NAME}.pot)


add_custom_command (
	OUTPUT
		${POT_FILE}
	COMMAND
		${XGETTEXT_EXECUTABLE} --output=${POT_FILE}
		--keyword=_ --keyword=N_ --keyword=C_:1c,2
		--directory=${CMAKE_SOURCE_DIR} --from-code=UTF-8
		--msgid-bugs-address=lxndr87@users.sourceforge.net
		src/client.c
		src/gstool.c
		src/gui-window.c
		src/gui-server-list.c
		src/gui-player-list.c
		src/gui-buddy-list.c
		src/gui-console.c
		src/gui-log.c
		src/gui-preferences.c
		src/gui-info.c
	DEPENDS
		../src/client.c
		../src/gstool.c
		../src/gui-window.c
		../src/gui-server-list.c
		../src/gui-player-list.c
		../src/gui-buddy-list.c
		../src/gui-console.c
		../src/gui-log.c
		../src/gui-preferences.c
		../src/gui-info.c
	COMMENT
		"Generating PO template file")


foreach (lang ${GS_LANGUAGES})
	set (PO_FILE ${lang}.po)
	set (MO_FILE ${lang}.mo)
	
	add_custom_command (
		OUTPUT ${MO_FILE}
		COMMAND ${MSGMERGE_EXECUTABLE} --update --quiet ${PO_FILE} ${POT_FILE}
		COMMAND ${MSGFMT_EXECUTABLE} --output=${MO_FILE} ${PO_FILE}
		DEPENDS ${POT_FILE} ${PO_FILE}
		COMMENT "Compiling ${lang} locale")
	
	install (
		FILES ${MO_FILE}
		DESTINATION ${CMAKE_INSTALL_FULL_LOCALEDIR}/${lang}/LC_MESSAGES
		RENAME ${CMAKE_PROJECT_NAME}.mo)
	
	set (MO_FILES ${MO_FILES} ${MO_FILE})
endforeach ()


add_custom_target (i18n ALL DEPENDS ${MO_FILES})
