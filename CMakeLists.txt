cmake_minimum_required( VERSION 2.6 )
find_package( OpenCV REQUIRED )

find_package( LipiTk REQUIRED
              HINTS /home/spe/projects/lipi/core/build_release/install )

MESSAGE( STATUS "\n\n" "LipiTk_DIR: " ${LipiTk_DIR} "\n" )
# MESSAGE( STATUS "\n\n" "LIPITK_LIBDIR: " ${LIPITK_LIBDIR} "\n" )

project( blackboard )

# Sources
set(blackboard_SRCS blackboard.cpp)

# # Includes
# set( LIPI_ROOT  $ENV{LIPI_ROOT})
# set( LIPITK_SRC ${LIPI_ROOT}/src)
# set( LIPITK_SRC_INCLUDE ${LIPITK_SRC}/include)
# set( LIPITK_SRC_UTILS ${LIPITK_SRC}/util)
# set( LIPITK_SRC_UTILS_LIB ${LIPITK_SRC_UTILS}/lib)
# set( LIPITK_LIPIENGINE ${LIPITK_SRC}/lipiengine)
# set( INC ${LIPITK_SRC_INCLUDE} ${LIPITK_SRC_UTILS_LIB} ${LIPITK_LIPIENGINE} )
# 
# # Libs
# set( LIPITK_LIB ${LIPI_ROOT}/lib)
# set( LIPITK_STATIC_LIBDIR ${LIPITK_SRC}/lib)
# set( LIB_DIRS ${LIPITK_LIB} ${LIPITK_STATIC_LIBDIR} )


include_directories(SYSTEM ${LIPITK_INCLUDE_DIRS} )
link_directories( ${LIPITK_LIBRARIES_DIRS} )

add_executable( blackboard ${blackboard_SRCS} )

target_link_libraries( blackboard ${OpenCV_LIBS} ${LIPITK_LIBRARIES} )

