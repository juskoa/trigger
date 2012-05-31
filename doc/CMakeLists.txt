#
# build the documentation
#

option ( BUILD_DOC "Build library and application documentation" ON)

# setup directories
if ( BUILD_DOC )
  if ( NOT DOC_INSTALL_DIR )
    set ( DOC_INSTALL_DIR share/doc/html
      CACHE PATH "Where to install generic documentation files to" FORCE )
  endif ( NOT DOC_INSTALL_DIR )
  mark_as_advanced ( DOC_INSTALL_DIR )

  if ( NOT MANPAGE_INSTALL_DIR )
    set ( MANPAGE_INSTALL_DIR share/man
      CACHE PATH "Where to install manpage files to" FORCE )
  endif ( NOT MANPAGE_INSTALL_DIR )
  mark_as_advanced ( MANPAGE_INSTALL_DIR )

  add_custom_target ( doc ALL )

  find_package ( Docbook )
  if ( Docbook_FOUND )
    include ( ${Docbook_USE_FILE} )
    option (BUILD_PEAC_APPS_HELP_MAN "Build manpage application help files" ON)
    option (BUILD_PEAC_APPS_HELP_HTML "Build HTML application help files" ON)
    option (BUILD_PEAC_DOC_HTML "Build HTML PEAC documentation" ON)
    add_subdirectory ( trigger-doc )
  else ( Docbook_FOUND )
    MESSAGE(STATUS "Docbook was NOT found ...")
  endif ( Docbook_FOUND )

endif ( BUILD_DOC )
