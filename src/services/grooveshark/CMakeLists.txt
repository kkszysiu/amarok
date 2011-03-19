remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)


include_directories(
        ../
        ../../
        ../../core-impl/collections
        ../../statusbar
        ../../widgets
        ../../context
        ../../network
        ../../dynamic # for CustomBias.h
        ../../browsers/playlistbrowser # for DynamicBiasWidgets
        ${CMAKE_CURRENT_BINARY_DIR}/../.. #for amarokconfig.h
        ${KDE4_INCLUDE_DIR}
        ${STRIGI_INCLUDE_DIR}
        ${QT_INCLUDES}
        #${LIBLASTFM_INCLUDE_DIR}/.. # so lastfm/foo/bar.h references work
        #${LIBLASTFM_INCLUDE_DIR}
        )

        add_subdirectory( images )

########### next target ###############

        add_definitions(${KDE4_ENABLE_EXCEPTIONS})

        set(amarok_service_grooveshark_PART_SRCS
            GroovesharkService.cpp
            GroovesharkServiceCollection.cpp
            GroovesharkServiceConfig.cpp
            GroovesharkServiceQueryMaker.cpp
            #LastfmReadLabelCapability.cpp
            #ScrobblerAdapter.cpp
            GroovesharkTreeModel.cpp
            GroovesharkTreeView.cpp
            AvatarDownloader.cpp

            meta/GroovesharkMeta.cpp
            meta/GroovesharkStreamInfoCapability.cpp

            biases/GroovesharkBias.cpp
            biases/WeeklyTopBias.cpp

            #SimilarArtistsAction.cpp
            LoveTrackAction.cpp
        )

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
        remove_definitions(-DQT_NO_HTTP)

        kde4_add_plugin(amarok_service_grooveshark ${amarok_service_grooveshark_PART_SRCS})
        target_link_libraries(amarok_service_grooveshark
            amarokcore
            amaroklib
            amarokpud
#${LIBLASTFM_LIBRARY}
            ${KDE4_KDECORE_LIBS}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KIO_LIBS}
            ${KDE4_SOLID_LIBRARY}
            ${KDE4_THREADWEAVER_LIBRARIES}
            ${QT_QTNETWORK_LIBRARY}
        )

        install(TARGETS amarok_service_grooveshark DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

        set(kcm_amarok_service_grooveshark_PART_SRCS
            GroovesharkServiceSettings.cpp
            GroovesharkServiceConfig.cpp
        )

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

        kde4_add_ui_files( kcm_amarok_service_grooveshark_PART_SRCS GroovesharkConfigWidget.ui )


        kde4_add_plugin( kcm_amarok_service_grooveshark ${kcm_amarok_service_grooveshark_PART_SRCS} )

        target_link_libraries( kcm_amarok_service_grooveshark
            amarokcore
            amaroklib
            #${LIBLASTFM_LIBRARY}
            ${KDE4_KDEUI_LIBS}
            ${KDE4_KUTILS_LIBS}
            ${KDE4_KIO_LIBS}
            ${QT_QTNETWORK_LIBRARY} )

        install(TARGETS kcm_amarok_service_grooveshark  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

        install( FILES amarok_service_grooveshark.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
        install( FILES amarok_service_grooveshark_config.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
        install(FILES amarokgrooveshark.protocol DESTINATION ${SERVICES_INSTALL_DIR})

