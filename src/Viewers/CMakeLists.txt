set(HEADERS Viewer.h
            CgnsViewer.h
            TecplotViewer.h)

set(SOURCE  Viewer.cpp
            CgnsViewer.cpp
            TecplotViewer.cpp)

add_library(Viewers ${HEADERS} ${SOURCE})
target_link_libraries(Viewers boost_system boost_filesystem cgns teciompi)
