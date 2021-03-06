include_directories( ../
                     ${QT_INCLUDES}             
                     ${TAGLIB_INCLUDES}         
                   )                            

add_definitions(${TAGLIB_CFLAGS})

set(amarok_afttagger_SRCS
    SafeFileSaver.cpp
    AFTTagger.cpp
)

add_executable(amarok_afttagger ${amarok_afttagger_SRCS} )

if( TAGLIB-EXTRAS_FOUND )
    include_directories(${TAGLIB-EXTRAS_INCLUDES})
    add_definitions(${TAGLIB-EXTRAS_CFLAGS})
    target_link_libraries(amarok_afttagger ${TAGLIB-EXTRAS_LIBRARIES})
endif( TAGLIB-EXTRAS_FOUND )

target_link_libraries(amarok_afttagger
                        ${QT_QTCORE_LIBRARY}
                        ${TAGLIB_LIBRARIES}
                     )


if(APPLE)
    SET_TARGET_PROPERTIES(amarok_afttagger PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
    # install to app bundle on os x, otherwise amarok fails to load it
    install(TARGETS amarok_afttagger DESTINATION ${BUNDLE_INSTALL_DIR}/Amarok.app/Contents/MacOS )
else(APPLE)
    install(TARGETS amarok_afttagger RUNTIME DESTINATION ${BIN_INSTALL_DIR} )
endif(APPLE)
