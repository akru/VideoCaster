# Search dependences
find_package (V4L2 REQUIRED)
include_directories (${V4L2_INCLUDE_DIRS})

find_package (libjpeg REQUIRED)
include_directories (${LIBJPEG_INCLUDE_DIRS})

find_package (Libmemcached REQUIRED)
include_directories (${LIBMEMCACHED_INCLUDE_DIR})

find_package (Threads REQUIRED)

# Video caster
set (vcast_SRCS vcast.c vgrab.c jcomp.c)
add_executable (vcast ${vcast_SRCS})
target_link_libraries (vcast ${V4L2_LIBRARY})
target_link_libraries (vcast ${LIBJPEG_LIBRARIES})

# Video server
set (vserv_SRCS vserv.c)
add_executable (vserv ${vserv_SRCS})
target_link_libraries (vserv ${LIBMEMCACHED_LIBRARY})
target_link_libraries (vserv ${CMAKE_THREAD_LIBS_INIT})
