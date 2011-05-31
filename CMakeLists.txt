cmake_minimum_required( VERSION 2.6 )

find_package( OpenCV REQUIRED )
find_package( LipiTk REQUIRED )


project( blackboard )

# includes (SYSTEM to avoid LIPITK warnings)
include_directories( SYSTEM ${LIPITK_INCLUDE_DIRS} )

# sources
set( sources blackboard.cpp)

# link_directories
link_directories( ${LIPITK_LIBRARIES_DIRS} )

# blackboard binary
add_executable( blackboard ${sources} )

# link libraries
target_link_libraries( blackboard ${OpenCV_LIBS} ${LIPITK_LIBRARIES} )
