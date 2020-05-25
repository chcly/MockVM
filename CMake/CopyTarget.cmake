


macro(copy_target TARNAME DESTDIR)

    add_custom_command(TARGET ${TARNAME} 
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARNAME}> 
                       "${DESTDIR}/$<TARGET_FILE_NAME:${TARNAME}>"
                       )

    if (MSVC)
        set_target_properties(
            ${TARNAME} 
            PROPERTIES 
            VS_DEBUGGER_COMMAND  
           "${DESTDIR}/$<TARGET_FILE_NAME:${TARNAME}>"
        )
    endif()

endmacro(copy_target)



macro(copy_install_target TARNAME)

    if (ToyVM_INSTALL_PATH)
        if (NOT ${ARGN} STREQUAL "")
            add_custom_command(TARGET ${TARNAME} 
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARNAME}> 
                               "${ToyVM_INSTALL_PATH}/${ARGN}/$<TARGET_FILE_NAME:${TARNAME}>"
                               )
        else()
            add_custom_command(TARGET ${TARNAME} 
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARNAME}> 
                               "${ToyVM_INSTALL_PATH}/$<TARGET_FILE_NAME:${TARNAME}>"
                               )
        endif()
    endif()

endmacro(copy_install_target)
