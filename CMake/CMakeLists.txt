# Utility project to organize build scripts better
if (MSVC)
    file(GLOB CM_FILES *.cmake)
    file(GLOB MD_FILES ../*.md)
    file(GLOB WF_FILES ../.github/workflows/*.yml)

    set(CMAKE_FILES
        ${CM_FILES}
        ${MD_FILES}
        ${WF_FILES}
        ../.clang-format
    )
 
    source_group("CMake"        FILES ${CM_FILES}) 
    source_group("Markdown"     FILES ${MD_FILES}) 
    source_group("GitHub"       FILES ${WF_FILES})

    set_source_files_properties(
        ${CMAKE_FILES} 
        PROPERTIES 
        VS_TOOL_OVERRIDE 
        "None"
    )

    add_custom_target(ALL_CMAKE ALL SOURCES ${CMAKE_FILES})
endif()
