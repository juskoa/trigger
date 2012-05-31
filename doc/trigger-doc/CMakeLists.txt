#
# build the documentation
#

if ( BUILD_DOC )
    set ( PEAC_DOC_XML_FILE "${CMAKE_CURRENT_SOURCE_DIR}/trigger-oncall-doc.xml" )
    if ( BUILD_PEAC_DOC_HTML )
      docbook_generate ( html "${PEAC_DOC_XML_FILE}" XSLT_FILES_HTML )
      message(STATUS "Html files : ${XSLT_FILES_HTML}")
      foreach ( file ${XSLT_FILES_HTML} )
	list ( APPEND PEAC_DOC_HELP_FILES ${file})
	install (
	  FILES ${CMAKE_CURRENT_BINARY_DIR}/${file}
	  DESTINATION ${DOC_INSTALL_DIR}
	  PERMISSIONS OWNER_READ OWNER_WRITE
		      GROUP_READ
		      WORLD_READ
	  COMPONENT documentation
	  OPTIONAL
	)
      endforeach ( file )
      install (
   FILES ${CMAKE_CURRENT_SOURCE_DIR}/docbook.css ${CMAKE_CURRENT_SOURCE_DIR}/trigger.css
   DESTINATION ${DOC_INSTALL_DIR}
   PERMISSIONS OWNER_READ OWNER_WRITE
        GROUP_READ
        WORLD_READ
   COMPONENT documentation
   OPTIONAL
      )

    endif ( BUILD_PEAC_DOC_HTML )
  # if there are some help files add it to deps
  if ( PEAC_DOC_HELP_FILES )
    add_custom_target ( trigger-oncall-doc DEPENDS ${PEAC_DOC_HELP_FILES} )
    add_dependencies ( doc trigger-oncall-doc )
  endif ( PEAC_DOC_HELP_FILES )

endif ( BUILD_DOC )
