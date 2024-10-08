project(k3b_src)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${libk3b_BINARY_DIR}
  ${libk3b_SOURCE_DIR}/core
  ${libk3b_SOURCE_DIR}/plugin
  ${libk3b_SOURCE_DIR}/tools
  ${libk3b_SOURCE_DIR}/tools/legacy
  ${libk3b_SOURCE_DIR}/projects
  ${libk3b_SOURCE_DIR}/projects/audiocd
  ${libk3b_SOURCE_DIR}/projects/datacd
  ${libk3b_SOURCE_DIR}/projects/videocd
  ${libk3b_SOURCE_DIR}/projects/mixedcd
  ${libk3b_SOURCE_DIR}/projects/movixcd/
  ${libk3b_SOURCE_DIR}/projects/videodvd/
  ${libk3b_SOURCE_DIR}/jobs
  ${libk3b_SOURCE_DIR}/videodvd
  ${libk3b_BINARY_DIR}/core
  ${libk3b_BINARY_DIR}/plugin
  ${libk3b_BINARY_DIR}/tools
  ${libk3b_BINARY_DIR}/projects
  ${libk3b_BINARY_DIR}/jobs
  ${CMAKE_CURRENT_SOURCE_DIR}/projects
  ${CMAKE_CURRENT_SOURCE_DIR}/projects/kostore/
  ${libk3b_SOURCE_DIR}/cddb
  )

add_subdirectory( pics )
add_subdirectory( icons )
add_subdirectory( services )


########### next target ###############

set(k3b_project_SRCS
  projects/k3baudioburndialog.cpp
  projects/k3baudiocdtextwidget.cpp
  projects/k3baudioeditorwidget.cpp
  projects/k3baudiotrackdialog.cpp
  projects/k3baudiotrackwidget.cpp
  #projects/k3baudiotrackplayer.cpp
  projects/k3baudiotracksplitdialog.cpp
  projects/k3baudioview.cpp
  projects/k3baudioviewimpl.cpp
  projects/k3baudioprojectmodel.cpp
  projects/k3bmixedburndialog.cpp
  projects/k3bmixedview.cpp
  projects/k3bdataadvancedimagesettingsdialog.cpp
  projects/k3bdatadirtreeview.cpp
  projects/k3bdataimagesettingswidget.cpp
  projects/k3bdatapropertiesdialog.cpp
  projects/k3bdataview.cpp
  projects/k3bdataviewimpl.cpp
  projects/k3bdatavolumedescdialog.cpp
  projects/k3bdataburndialog.cpp
  projects/k3bdataprojectmodel.cpp
  projects/k3bbootimagedialog.cpp
  projects/k3bbootimageview.cpp
  projects/k3bbootimagemodel.cpp
  projects/k3bmovixburndialog.cpp
  projects/k3bmovixoptionswidget.cpp
  projects/k3bmovixview.cpp
  projects/k3bmovixprojectmodel.cpp
  projects/k3bvideodvdburndialog.cpp
  projects/k3bvideodvdview.cpp
  projects/k3bvcdburndialog.cpp
  projects/k3bvcdtrackdialog.cpp
  projects/k3bvcdtrackkeysdelegate.cpp
  projects/k3bvcdtrackkeysmodel.cpp
  projects/k3bvcdview.cpp
  projects/k3bvcdprojectmodel.cpp
  projects/k3bfillstatusdisplay.cpp
  projects/k3bview.cpp
  projects/k3bdirproxymodel.cpp
  projects/k3bprojectburndialog.cpp
  projects/k3bprojectplugindialog.cpp
  projects/k3bdatamultisessioncombobox.cpp
  projects/k3bdataurladdingdialog.cpp
  projects/k3baudiodatasourceeditwidget.cpp
  projects/k3baudiotrackaddingdialog.cpp
  projects/k3bencodingconverter.cpp
  projects/k3bdatamultisessionimportdialog.cpp
  projects/k3bvolumenamewidget.cpp
  )

kde4_add_ui_files(k3b_project_SRCS
  projects/base_k3baudiocdtextwidget.ui
  projects/base_k3bbootimageview.ui
  projects/base_k3badvanceddataimagesettings.ui
  projects/base_k3baudiocdtextallfieldswidget.ui
  projects/base_k3bdataimagesettings.ui
  projects/base_k3bdatavolumedescwidget.ui
  projects/base_k3baudiotrackwidget.ui
  projects/base_k3bmovixoptionswidget.ui
  )

set(k3b_kostore_SRCS
  projects/kostore/KoStore.cpp
  projects/kostore/KoZipStore.cpp
  projects/kostore/KoStoreBase.cpp
  )

set(k3b_rip_SRCS
  rip/k3baudioripjob.cpp
  rip/k3bpatternparser.cpp
  rip/k3baudiorippingdialog.cpp
  rip/k3baudiocdview.cpp
  rip/k3bcddbpatternwidget.cpp
  rip/k3bvideocdinfo.cpp
  rip/k3bvideocdview.cpp
  rip/k3bvideocdrip.cpp
  rip/k3bvideocdrippingdialog.cpp
  rip/k3bcuefilewriter.cpp
  rip/k3baudioconvertingoptionwidget.cpp
  rip/k3baudioprojectconvertingdialog.cpp
  rip/k3baudioprojectconvertingjob.cpp
  rip/categories.cpp
  rip/k3baudiotrackmodel.cpp
  rip/k3bviewcolumnadjuster.cpp
  )

kde4_add_ui_files(k3b_rip_SRCS
  rip/base_k3baudiorippingoptionwidget.ui
  rip/base_k3bcddbpatternwidget.ui
  )

set(k3b_option_SRCS
  option/k3bmiscoptiontab.cpp
  option/k3bexternalbinmodel.cpp
  option/k3bexternalbinoptiontab.cpp
  option/k3bexternalbinparamsmodel.cpp
  option/k3badvancedoptiontab.cpp
  option/k3boptiondialog.cpp
  option/k3bdeviceoptiontab.cpp
  option/k3bnotifyoptiontab.cpp
  option/k3bpluginoptiontab.cpp
  option/k3bthemeoptiontab.cpp
  option/k3bthememodel.cpp
  option/k3bdevicewidget.cpp
  option/k3bexternalbinwidget.cpp
  option/k3bcddboptiontab.cpp
  )

kde4_add_ui_files(k3b_option_SRCS
  option/base_k3bthemeoptiontab.ui
  option/base_k3bmiscoptiontab.ui
  option/base_k3bpluginoptiontab.ui
  )

set(k3b_misc_SRCS
  misc/k3bimagewritingdialog.cpp
  misc/k3bmediacopydialog.cpp
  misc/k3bmediaformattingdialog.cpp
  )

set(k3b_SRCS
  k3bwelcomewidget.cpp
  k3bapplication.cpp
  k3bdevicedelegate.cpp
  k3bmediumdelegate.cpp
  k3bmetaitemmodel.cpp
  k3bplacesmodel.cpp
  k3bdiroperator.cpp
  k3bfiletreeview.cpp
  k3bprojecttabwidget.cpp
  k3bsplash.cpp
  k3bfileview.cpp
  k3bdirview.cpp
  k3b.cpp
  main.cpp
  k3bstatusbarmanager.cpp
  k3burlnavigator.cpp
  k3binterface.cpp
  k3bjobinterface.cpp
  k3baudioprojectinterface.cpp
  k3bdataprojectinterface.cpp
  k3bmixedprojectinterface.cpp
  k3bprojectinterface.cpp
  k3bsystemproblemdialog.cpp
  k3bmediacontentsview.cpp
  k3bcontentsview.cpp
  k3bjobprogressdialog.cpp
  k3bburnprogressdialog.cpp
  k3btempdirselectionwidget.cpp
  k3bdatamodewidget.cpp
  k3bwritingmodewidget.cpp
  k3bwriterselectionwidget.cpp
  k3binteractiondialog.cpp
  k3bthememanager.cpp
  k3bprojectmanager.cpp
  k3bflatbutton.cpp
  k3bemptydiscwaiter.cpp
  k3bjobprogressosd.cpp
  k3bdebuggingoutputcache.cpp
  k3bdebuggingoutputdialog.cpp
  k3bdebuggingoutputfile.cpp
  k3bappdevicemanager.cpp
  k3bmediaselectioncombobox.cpp
  k3bwidgetshoweffect.cpp
  k3bmediaselectiondialog.cpp
  k3bdiskinfoview.cpp
  k3bpassivepopup.cpp
  k3btimeoutwidget.cpp
  k3bminibutton.cpp
  k3bthemedheader.cpp
  k3bthemedlabel.cpp
  k3blsofwrapper.cpp
  k3blsofwrapperdialog.cpp
  k3baction.cpp
  k3bdevicemenu.cpp
  )

qt4_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/k3binterface.h org.k3b.MainWindow.xml )
qt4_add_dbus_adaptor( k3b_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.k3b.MainWindow.xml k3binterface.h K3b::Interface k3binterfaceadaptor K3bInterfaceAdaptor )
qt4_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/k3bjobinterface.h org.k3b.Job.xml )
qt4_add_dbus_adaptor( k3b_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.k3b.Job.xml k3bjobinterface.h K3b::JobInterface k3bjobinterfaceadaptor K3bJobInterfaceAdaptor )
qt4_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/k3bprojectinterface.h org.k3b.Project.xml )
qt4_add_dbus_adaptor( k3b_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.k3b.Project.xml k3bprojectinterface.h K3b::ProjectInterface k3bprojectinterfaceadaptor K3bProjectInterfaceAdaptor )
qt4_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/k3baudioprojectinterface.h org.k3b.AudioProject.xml )
qt4_add_dbus_adaptor( k3b_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.k3b.AudioProject.xml k3baudioprojectinterface.h K3b::AudioProjectInterface k3baudioprojectinterfaceadaptor K3bAudioProjectInterfaceAdaptor )
qt4_generate_dbus_interface( ${CMAKE_CURRENT_SOURCE_DIR}/k3bdataprojectinterface.h org.k3b.DataProject.xml )
qt4_add_dbus_adaptor( k3b_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.k3b.DataProject.xml k3bdataprojectinterface.h K3b::DataProjectInterface k3bdataprojectinterfaceadaptor K3bDataProjectInterfaceAdaptor )

set(k3b_bin_SRCS
  ${k3b_option_SRCS}
  ${k3b_misc_SRCS}
  ${k3b_project_SRCS}
  ${k3b_kostore_SRCS}
  ${k3b_rip_SRCS}
  ${k3b_dbus_SRCS}
  ${k3b_SRCS}
)

if(ENABLE_DVD_RIPPING)
  set(k3b_videodvd_SRCS
    rip/videodvd/k3bvideodvdaudiomodel.cpp
    rip/videodvd/k3bvideodvdrippingview.cpp
    rip/videodvd/k3bvideodvdrippingjob.cpp
    rip/videodvd/k3bvideodvdrippingwidget.cpp
    rip/videodvd/k3bvideodvdrippingdialog.cpp
    rip/videodvd/k3bvideodvdrippingpreview.cpp
    rip/videodvd/k3bvideodvdtitledelegate.cpp
    rip/videodvd/k3bvideodvdtitlemodel.cpp
    )

  kde4_add_ui_files(k3b_videodvd_SRCS
    rip/videodvd/base_k3bvideodvdrippingwidget.ui)

  set(k3b_bin_SRCS
    ${k3b_bin_SRCS}
    ${k3b_videodvd_SRCS}
    )
endif(ENABLE_DVD_RIPPING)

if(ENABLE_MUSICBRAINZ)
  set(k3b_SRCS
    ${k3b_SRCS}
    k3btrm.cpp
    k3bmusicbrainz.cpp
    )
  set(k3b_project_SRCS
    ${k3b_project_SRCS}
    projects/k3bmusicbrainzjob.cpp
    projects/k3bmusicbrainztrackloopupjob.cpp
    projects/k3baudiotracktrmlookupdialog.cpp
    )
endif(ENABLE_MUSICBRAINZ)

kde4_add_app_icon(k3b_bin_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/icons/hi*-app-k3b.png)

if (MSVC)
    # msvc creates importlibs by default also for executables when symbols are exported.
    # Because the output name of the executable is the same as one of the used library (k3b)  creating
    # the importlibrary fails with LINK : fatal error LNK1149: output filename matches input filename
    # we worked around this issue by using a different name for the unused import library created by the linker
    string(REPLACE "/implib:<TARGET_IMPLIB>" "/implib:<TARGET_IMPLIB>_" CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE})
endif (MSVC)

kde4_add_executable(k3b_bin ${k3b_bin_SRCS})

set_target_properties(k3b_bin PROPERTIES OUTPUT_NAME k3b)

target_link_libraries(k3b_bin
  k3bdevice
  k3b
  ${KCDDB_LIBRARIES}
  ${KDE4_KFILE_LIBS}
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDE4_KNOTIFYCONFIG_LIBS}
  ${KDE4_KUTILS_LIBS}
  ${KDE4_SOLID_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  ${X11_LIBRARIES}
  )

if(ENABLE_MUSICBRAINZ)
  target_link_libraries(k3b_bin ${MUSICBRAINZ_LIBRARIES})
endif(ENABLE_MUSICBRAINZ)

install(TARGETS k3b_bin ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES  k3b.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
#install( FILES  k3b-iso.desktop k3b-cue.desktop DESTINATION ${APPLNK_INSTALL_DIR}/.hidden/ )
install( FILES  k3bui.rc k3bdeviceui.rc k3b.notifyrc tips DESTINATION ${DATA_INSTALL_DIR}/k3b )

set( SHARED_MIME_INFO_MINIMUM_VERSION "0.23" )
find_package( SharedMimeInfo REQUIRED )
install( FILES x-k3b.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
