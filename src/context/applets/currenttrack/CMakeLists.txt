project(context-currenttrack)

include_directories(
    ${Amarok_SOURCE_DIR}/src/context/widgets
    ${Amarok_SOURCE_DIR}/src/core-impl/collections/support
    ${Amarok_SOURCE_DIR}/src
    )

set( currenttrack_SRCS
     CurrentTrack.cpp
     ${Amarok_SOURCE_DIR}/src/context/widgets/RecentlyPlayedListWidget.cpp
     ${Amarok_SOURCE_DIR}/src/widgets/PixmapViewer.cpp
     )

kde4_add_ui_files( currenttrack_SRCS currentTrackSettings.ui )
kde4_add_plugin(amarok_context_applet_currenttrack ${currenttrack_SRCS})
if(APPLE)
   SET_TARGET_PROPERTIES(amarok_context_applet_currenttrack PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)
target_link_libraries(amarok_context_applet_currenttrack amarokcore amaroklib ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS amarok_context_applet_currenttrack DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES amarok-context-applet-currenttrack.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES amarok-currenttrack.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/ )
