file(GLOB jsoncpp_source *.cpp)
file(GLOB jsoncpp_header *.h)


add_library(json SHARED ${jsoncpp_source})

install(FILES ${jsoncpp_header} DESTINATION include/json) 
install(TARGETS json LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
