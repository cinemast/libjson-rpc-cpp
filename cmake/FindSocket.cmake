# - This module determines the socket library of the system.
# The following variables are set
#  Socket_LIBRARIES     - the socket library
#  Socket_FOUND

if(${CMAKE_SYSTEM} MATCHES "Windows")
  SET(Socket_LIBRARIES wsock32 ws2_32)
  SET(Socket_FOUND 1)
elseif(${CMAKE_SYSTEM} MATCHES "INtime")
  SET(Socket_LIBRARIES netlib)
  SET(Socket_FOUND 1)
else()
  SET(Socket_LIBRARIES)
  SET(Socket_FOUND 1)
endif()
