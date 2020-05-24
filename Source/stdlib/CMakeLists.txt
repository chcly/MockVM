

set(SRC
   stdlib.cpp
)


include_directories(../)


add_library(std    ${SRC})
add_library(libstd SHARED ${SRC})


target_link_libraries(std    libtvm)
target_link_libraries(libstd libtvm)


if (NOT INSTALL_PATH STREQUAL "")

add_custom_command(TARGET libstd 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libstd> 
                    "${INSTALL_PATH}/lib/$<TARGET_FILE_BASE_NAME:libstd>$<TARGET_FILE_SUFFIX:libstd>"
                    )


endif()

add_custom_command(TARGET libstd 
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:libstd> 
                    "${LIB_DIR}/$<TARGET_FILE_BASE_NAME:libstd>$<TARGET_FILE_SUFFIX:libstd>"
                    )

