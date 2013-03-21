
add_subdirectory(json)

if(USE_HTTP)
  add_subdirectory(mongoose)
  set(DEP_LIBS ${DEP_LIBS} mongoose)
endif()

add_subdirectory(jsonrpc)

