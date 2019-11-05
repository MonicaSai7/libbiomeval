# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
#
# Created by NIST for the Biometric Evaluation Framework.
#
#.rst:
# FindBerkeleyDB
# --------
#
# Find BerkeleyDB 
#
# Find the Berkeley database C++ includes and library This module defines
#
# ::
#
#   BERKELEYDB_INCLUDE_DIR, where to find db_cxx.h, etc.
#   BERKELEYDB_LIBRARIES, the libraries needed to use BDB.
#   BERKELEYDB_FOUND, If false, do not try to use BDB.
#
# also defined, but not for general use are
#
# ::
#
#   BERKELEYDB_LIBRARY, where to find the library.

find_path(BERKELEYDB_INCLUDE_DIR db_cxx.h
  /usr/include/
  /opt/local/include/
)

set(BERKELEYDB_NAMES ${BERKELEYDB_NAMES} db_cxx)
find_library(BERKELEYDB_LIBRARY NAMES ${BERKELEYDB_NAMES})

# handle the QUIETLY and REQUIRED arguments and set BERKELEYDB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BERKELEYDB DEFAULT_MSG BERKELEYDB_LIBRARY BERKELEYDB_INCLUDE_DIR)

if(BERKELEYDB_FOUND)
  set(BERKELEYDB_LIBRARIES ${BERKELEYDB_LIBRARY})
endif()

mark_as_advanced(BERKELEYDB_LIBRARY BERKELEYDB_INCLUDE_DIR)
