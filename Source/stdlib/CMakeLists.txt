

set(SRC
   stdlib.cpp
)


include_directories(../)
add_library(std ${SRC})
target_link_libraries(std libtvm)
