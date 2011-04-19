include_directories(
    ../
    ../../
    ../../core-impl/collections
    ../../servicebrowser
    ../../statusbar
    ${CMAKE_CURRENT_BINARY_DIR}/../../..
    ${KDE4_INCLUDE_DIR}
    ${STRIGI_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${QJSON_INCLUDE_DIR}
    grooveshark
)

add_subdirectory( images )

########### next target ###############

set(amarok_service_grooveshark_PART_SRCS
    grooveshark/QGrooveshark.cpp
    grooveshark/QGroovesharkPlaylist.cpp
    grooveshark/QGroovesharkSearch.cpp
    grooveshark/QGroovesharkSong.cpp
    GroovesharkActions.cpp
    GroovesharkAlbumDownloader.cpp
    GroovesharkCollectionLocation.cpp
    GroovesharkConfig.cpp
    GroovesharkDatabaseHandler.cpp
    GroovesharkDatabaseWorker.cpp
    GroovesharkDownloadDialog.cpp
    GroovesharkDownloadInfo.cpp
    GroovesharkInfoParser.cpp
    GroovesharkMeta.cpp
    GroovesharkDownloadHandler.cpp
    GroovesharkRedownloadDialog.cpp
    GroovesharkRedownloadHandler.cpp
    GroovesharkCollection.cpp
    GroovesharkStore.cpp
    GroovesharkServiceModel.cpp
    GroovesharkServiceView.cpp
    GroovesharkTreeItem.cpp
    GroovesharkPodcastTreeItem.cpp
    GroovesharkTagTreeItem.cpp
    GroovesharkSortFilterProxyModel.cpp
    GroovesharkUrlRunner.cpp
    GroovesharkXmlParser.cpp

)

kde4_add_ui_files( amarok_service_grooveshark_PART_SRCS
    GroovesharkDownloadDialogBase.ui
    GroovesharkRedownloadDialogBase.ui
    GroovesharkSignupDialogBase.ui

)

kde4_add_plugin(amarok_service_grooveshark ${amarok_service_grooveshark_PART_SRCS})
target_link_libraries(amarok_service_grooveshark
    amarokcore
    amaroklib
    amarokpud
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_THREADWEAVER_LIBRARIES}
    ${QJSON_LIBRARIES}
)


install(TARGETS amarok_service_grooveshark DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

set(kcm_amarok_service_grooveshark_PART_SRCSS
    GroovesharkSettingsModule.cpp
    GroovesharkConfig.cpp
)

kde4_add_ui_files( kcm_amarok_service_grooveshark_PART_SRCSS GroovesharkConfigWidget.ui )


kde4_add_plugin( kcm_amarok_service_grooveshark ${kcm_amarok_service_grooveshark_PART_SRCSS} )


target_link_libraries( kcm_amarok_service_grooveshark  ${KDE4_KUTILS_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${QJSON_LIBRARIES} )

install(TARGETS kcm_amarok_service_grooveshark  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES amarok_service_grooveshark.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES amarok_service_grooveshark_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
