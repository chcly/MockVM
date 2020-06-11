# -----------------------------------------------------------------------------
#   Copyright (c) 2020 Charles Carley.
#
#   This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
# ------------------------------------------------------------------------------

include_directories(../libtvm)

set(SRC
  Console.h
  Color.h
  Console.cpp
  Debugger.h
  Debugger.cpp
  Main.cpp
)

if (WIN32)
    set(PLAT_NO_COMP
      ConsoleCurses.h
      ConsoleCurses.cpp 
      Win32/ncurses.h
      Win32/unistd.h
     )

    set_source_files_properties(${PLAT_NO_COMP} PROPERTIES HEADER_FILE_ONLY ON)
    include_directories(Win32)

    set(PLAT
      ConsoleWindows.h
      ConsoleWindows.cpp
      ${PLAT_NO_COMP}
    )


else()

    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)

    include_directories(${CURSES_INCLUDE_DIRS})
    set(PLAT_LIBS ${CURSES_LIBRARIES})

    set(PLAT
      ConsoleCurses.h
      ConsoleCurses.cpp
    )
endif()

add_executable(tdbg  ${SRC} ${PLAT})
target_link_libraries(tdbg libtvm ${PLAT_LIBS})


copy_target(tdbg ${ToyVM_BIN_DIR})
copy_install_target(tdbg)

