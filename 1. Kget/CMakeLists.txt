project(kget)

if(NOT INSIDE_KDENETWORK)
    message("Not building inside KDENetwork, loading KDE CMake Macros.")

    find_package(KDE4 REQUIRED)

    include(KDE4Defaults)
    include(MacroLibrary)

    include(CheckIncludeFile)
    include(CheckIncludeFiles)
    include(CheckSymbolExists)
    include(CheckFunctionExists)
    include(CheckLibraryExists)
    include(CheckPrototypeExists)
    include(CheckTypeSize)

    set(CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS})
    if(WIN32)
       set(CMAKE_REQUIRED_LIBRARIES ${KDEWIN32_LIBRARIES})
       set(CMAKE_REQUIRED_INCLUDES  ${KDEWIN32_INCLUDES})
    endif(WIN32)
    add_definitions(${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
endif(NOT INSIDE_KDENETWORK)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

macro_optional_find_package(NepomukCore)
macro_log_feature(NepomukCore_FOUND "NepomukCore" "Semantic Desktop" "http://nepomuk.kde.org" FALSE "" "Nepomuk is needed for the integration into KGet")

macro_optional_find_package(NepomukWidgets)
macro_log_feature(NepomukWidgets_FOUND "NepomukWidgets" "Semantic Desktop" "http://nepomuk.kde.org" FALSE "" "Nepomuk is needed for the integration into KGet")

macro_optional_find_package(Sqlite)
macro_log_feature(SQLITE_FOUND "SQLite" "SQLite is a Binary-Database" "" FALSE "" "Needed for the SQLite-Backend of the KGet-History and the Kopete-Statistic-Plugin")

macro_optional_find_package(QCA2)
macro_log_feature(QCA2_FOUND "QCA2" "Qt Cryptographic Architecture" "http://delta.affinix.com/qca" FALSE "2.0.0" "Needed for the KGet bittorrent-plugin and some Kopete plugins")

macro_optional_find_package(QGpgme)
macro_log_feature(QGPGME_FOUND "QGpgME" "The QGpgME library" "http://www.kde.org" FALSE "" "QGpgME is required to have signature verifying support in KGet.")

macro_optional_find_package(KTorrent)
macro_log_feature(LIBKTORRENT_FOUND "libktorrent" "Backend Library of KTorrent" "http://ktorrent.org" FALSE "" "Needed to build KGet bittorrent support.")

macro_optional_find_package(LibMms)
macro_log_feature(LIBMMS_FOUND "libmms" "Library to read mms streams" "http://libmms.sourceforge.net" FALSE "" "Needed to build KGet mms support.")

set(kget_adaptor_SRCS
    dbus/dbuskgetwrapper.cpp
)
# set(kget_transfer_adaptor_SRCS
#     core/transferhandler.cpp
#     core/transfertreemodel.cpp
# )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5001) 

if(CMAKE_BUILD_TYPE MATCHES debugfull)
     add_definitions(-DDEBUG)
endif(CMAKE_BUILD_TYPE MATCHES debugfull)

if (NepomukCore_FOUND AND NepomukWidgets_FOUND)
    find_package(Soprano)
    find_package(SharedDesktopOntologies 0.4)
endif (NepomukCore_FOUND AND NepomukWidgets_FOUND)
if(SharedDesktopOntologies_FOUND)
    set(HAVE_NEPOMUK SharedDesktopOntologies_FOUND)
    include_directories(
        ${SOPRANO_INCLUDE_DIR}
        ${NEPOMUK_CORE_INCLUDE_DIR}
        ${NEPOMUK_WIDGETS_INCLUDE_DIR}
    )
    add_definitions(-DHAVE_NEPOMUK)
    add_definitions(-DDISABLE_NEPOMUK_LEGACY)
    include(NepomukAddOntologyClasses)
endif(SharedDesktopOntologies_FOUND)

if(QCA2_FOUND)
    add_definitions(-DHAVE_QCA2)
    include_directories(
        ${QCA2_INCLUDE_DIR}
    )
endif(QCA2_FOUND)

if(QGPGME_FOUND)
    find_package(Boost REQUIRED)
    add_definitions(-DHAVE_QGPGME)
    include_directories(
        ${Boost_INCLUDE_DIRS}
        ${QGPGME_INCLUDE_DIR}
    )
endif(QGPGME_FOUND)

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(extensions)
add_subdirectory(sounds)
add_subdirectory(desktop)
add_subdirectory(plasma)
add_subdirectory(tests)
add_subdirectory(doc)

macro_optional_find_package(KDE4Workspace)
macro_log_feature(KDE4WORKSPACE_FOUND "KDE4Workspace" "KDE4 workspace libraries, part of kdebase-workspace" "http://www.kde.org" FALSE "" "Allows 'shutdown after downloads completed' in kget")
if(KDE4WORKSPACE_FOUND)
    add_definitions(-DHAVE_KWORKSPACE)
	include_directories(${KDE4WORKSPACE_INCLUDE_DIR})
endif(KDE4WORKSPACE_FOUND)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(SQLITE_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif(SQLITE_FOUND)


remove_definitions(-DQT_NO_HTTP)

# kgetcore

set(kgetcore_SRCS
   core/job.cpp
   core/jobqueue.cpp
   core/kget.cpp
   core/scheduler.cpp
   core/transfertreemodel.cpp
   core/transfertreeselectionmodel.cpp
   core/transfer.cpp
   core/transfergroup.cpp
   core/transfergrouphandler.cpp
   core/transferhandler.cpp
   core/handler.cpp
   core/transfergroupscheduler.cpp
   core/plugin/plugin.cpp
   core/plugin/transferfactory.cpp
   core/transferdatasource.cpp
   core/datasourcefactory.cpp
   core/kgetkjobadapter.cpp
   core/kuiserverjobs.cpp
   core/kgetglobaljob.cpp
   core/bitset.cpp
   core/download.cpp
   core/transferhistorystore.cpp
   core/linkimporter.cpp
   dbus/dbustransferwrapper.cpp
   dbus/dbusverifierwrapper.cpp
   core/filemodel.cpp
   core/verifier.cpp
   core/verificationthread.cpp
   core/verificationmodel.cpp
   core/verificationdelegate.cpp
   core/signature.cpp
   core/signaturethread.cpp
   core/keydownloader.cpp
   core/urlchecker.cpp
   core/basedialog.cpp
   core/mostlocalurl.cpp
   core/filedeleter.cpp
)

if(HAVE_NEPOMUK)
    set(kgetcore_SRCS
       ${kgetcore_SRCS}
       core/nepomukhandler.cpp
       core/nepomukcontroller.cpp
    )

    NEPOMUK2_ADD_ONTOLOGY_CLASSES(
        kgetcore_SRCS
        ONTOLOGIES
        ${CMAKE_CURRENT_SOURCE_DIR}/kget_history.trig
    )
endif (HAVE_NEPOMUK)

kde4_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)

qt4_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.transfer.xml dbus/dbustransferwrapper.h DBusTransferWrapper)
qt4_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.verifier.xml dbus/dbusverifierwrapper.h DBusVerifierWrapper)


kde4_add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})

if (KDE4WORKSPACE_FOUND)
    target_link_libraries(kgetcore ${KDE4WORKSPACE_KWORKSPACE_LIBS})
endif (KDE4WORKSPACE_FOUND)

if (HAVE_NEPOMUK)
  target_link_libraries(kgetcore ${SOPRANO_LIBRARIES} ${NEPOMUK_CORE_LIBRARY} ${NEPOMUK_WIDGETS_LIBRARY})
endif (HAVE_NEPOMUK)

if (SQLITE_FOUND)
  target_link_libraries(kgetcore ${QT_QTSQL_LIBRARY})
endif (SQLITE_FOUND)

if (QCA2_FOUND)
    target_link_libraries(kgetcore ${QCA2_LIBRARIES})
endif (QCA2_FOUND)

if (QGPGME_FOUND)
    target_link_libraries(kgetcore ${QGPGME_LIBRARIES})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
endif(QGPGME_FOUND)

set_target_properties(kgetcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kgetcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# kget

set(kget_SRCS ${kget_adaptor_SRCS} ${kget_transfer_adaptor_SRCS}
   conf/autopastemodel.cpp
   conf/integrationpreferences.cpp
   conf/dlgwebinterface.cpp
   conf/preferencesdialog.cpp
   conf/transfersgrouptree.cpp
   conf/transfersgroupwidget.cpp
   conf/pluginselector.cpp
   conf/verificationpreferences.cpp
   ui/droptarget.cpp
   ui/transfersview.cpp
   ui/transfersviewdelegate.cpp
   ui/transferdetails.cpp
   ui/viewscontainer.cpp
   ui/newtransferdialog.cpp
   ui/groupsettingsdialog.cpp
   ui/transfersettingsdialog.cpp
   ui/contextmenu.cpp
   ui/tray.cpp
   ui/history/rangetreewidget.cpp
   ui/history/transferhistory.cpp
   ui/history/transferhistoryitemdelegate.cpp
   ui/history/transferhistorycategorizeddelegate.cpp
   ui/history/transferhistorycategorizedview.cpp
   ui/linkview/kget_linkview.cpp
   ui/linkview/kget_sortfilterproxymodel.cpp
   ui/mirror/mirrorsettings.cpp
   ui/mirror/mirrormodel.cpp
   ui/renamefile.cpp
   ui/verificationdialog.cpp
   ui/metalinkcreator/metalinkcreator.cpp
   ui/metalinkcreator/generalwidget.cpp
   ui/metalinkcreator/metalinker.cpp
   ui/metalinkcreator/filedlg.cpp
   ui/metalinkcreator/localemodels.cpp
   ui/metalinkcreator/dragdlg.cpp
   ui/metalinkcreator/urlwidget.cpp
   ui/metalinkcreator/filehandler.cpp
   ui/signaturedlg.cpp
   mainwindow.cpp
   main.cpp

   extensions/webinterface/httpserver.cpp
)

qt4_add_dbus_adaptor(kget_SRCS dbus/org.kde.kget.main.xml dbus/dbuskgetwrapper.h DBusKGetWrapper)

kde4_add_ui_files(kget_SRCS
   conf/dlgadvanced.ui
   conf/dlgappearance.ui
   conf/dlggroups.ui
   conf/dlgintegration.ui
   conf/dlgwebinterface.ui
   conf/dlgnetwork.ui
   conf/verificationpreferences.ui
   ui/transferdetailsfrm.ui
   ui/newtransferwidget.ui
   ui/history/transferhistory.ui
   ui/groupsettingsdialog.ui
   ui/transfersettingsdialog.ui
   ui/linkview/importlinkdialog.ui
   ui/mirror/mirrorsettings.ui
   ui/mirror/mirroradddlg.ui
   ui/renamefile.ui
   ui/verificationdialog.ui
   ui/verificationadddlg.ui
   ui/metalinkcreator/introduction.ui
   ui/metalinkcreator/generalwidget.ui
   ui/metalinkcreator/files.ui
   ui/metalinkcreator/filedlg.ui
   ui/metalinkcreator/dragdlg.ui
   ui/metalinkcreator/urlwidget.ui
   ui/metalinkcreator/commondata.ui
   ui/signaturedlg.ui
)

kde4_add_app_icon(kget_SRCS "ui/icons/hi*-apps-kget.png")

kde4_add_executable(kget ${kget_SRCS})

target_link_libraries(kget ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY} ${KDE4_KCMUTILS_LIBRARY} kgetcore ${KDE4_KNOTIFYCONFIG_LIBS})
if(QCA2_FOUND)
  target_link_libraries(kget ${QCA2_LIBRARIES})
endif(QCA2_FOUND)


if (HAVE_NEPOMUK)
  target_link_libraries(kget ${SOPRANO_LIBRARIES} ${NEPOMUK_CORE_LIBRARY} ${NEPOMUK_WIDGETS_LIBRARY})
endif (HAVE_NEPOMUK)

if (QGPGME_FOUND)
  target_link_libraries(kget ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)

install(TARGETS kget ${INSTALL_TARGETS_DEFAULT_ARGS})

if (HAVE_NEPOMUK)
    configure_file(kget_history.ontology.in ${CMAKE_CURRENT_BINARY_DIR}/kget_history.ontology)
    install(FILES kget_history.trig ${CMAKE_CURRENT_BINARY_DIR}/kget_history.ontology DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ontology/kde)
endif (HAVE_NEPOMUK)

if (NOT INSIDE_KDENETWORK)
    macro_display_feature_log()
endif ()
