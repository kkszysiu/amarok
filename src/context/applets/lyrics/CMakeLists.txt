project(context-currenttrack)

set(lyrics_SRCS
    LyricsApplet.cpp
    LyricsBrowser.cpp
    LyricsSuggestionsListWidget.cpp)

include_directories( ../..
                     ../../.. )

kde4_add_ui_files( lyrics_SRCS lyricsSettings.ui )
kde4_add_plugin(amarok_context_applet_lyrics ${lyrics_SRCS})
if(APPLE)
   SET_TARGET_PROPERTIES(amarok_context_applet_lyrics PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)
target_link_libraries(amarok_context_applet_lyrics amarokcore amaroklib ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS amarok_context_applet_lyrics DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES amarok-context-applet-lyrics.desktop DESTINATION ${SERVICES_INSTALL_DIR})
