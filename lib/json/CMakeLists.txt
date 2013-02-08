file(GLOB jsoncpp_source *.cpp)
file(GLOB jsoncpp_header *.h)


add_library(json SHARED ${jsoncpp_source})
add_library(jsonStatic STATIC ${jsoncpp_source})

set_target_properties(jsonStatic PROPERTIES OUTPUT_NAME json)


install(FILES ${jsoncpp_header} DESTINATION include/json) 
install(TARGETS json jsonStatic LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
