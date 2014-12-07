find_path(LIBOFA_INCLUDE_DIR NAMES ofa.h
   HINTS
   ~/usr/include
   /opt/local/include
   /usr/include
   /usr/local/include
   /opt/kde4/include
   ${KDE4_INCLUDE_DIR}
   PATH_SUFFIXES ofa1
)

find_library(LIBOFA_LIBRARY NAMES ofa
    PATHS
    ~/usr/lib
   /opt/local/lib
   /usr/lib
   /usr/lib64
   /usr/local/lib
   /opt/kde4/lib
   ${KDE4_LIB_DIR}
)


if(LIBOFA_INCLUDE_DIR AND LIBOFA_LIBRARY)
   set(LIBOFA_FOUND TRUE)
   message(STATUS "Found libofa: ${LIBOFA_INCLUDE_DIR}, ${LIBOFA_LIBRARY}")
else(LIBOFA_INCLUDE_DIR AND LIBOFA_LIBRARY)
   set(LIBOFA_FOUND FALSE)
   if (LIBOFA_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find required package LibOFA")
   endif(LIBOFA_FIND_REQUIRED)
endif(LIBOFA_INCLUDE_DIR AND LIBOFA_LIBRARY)

mark_as_advanced(LIBOFA_INCLUDE_DIR LIBOFA_LIBRARY)
