file(GLOB jsoncpp_source *.cpp)
file(GLOB jsoncpp_header *.h)

add_library(jsoncpp SHARED ${jsoncpp_source})
add_library(jsoncppStatic STATIC ${jsoncpp_source})

set_target_properties(jsoncppStatic PROPERTIES OUTPUT_NAME jsoncpp)


install(FILES ${jsoncpp_header} DESTINATION include/json) 
install(TARGETS jsoncpp jsoncppStatic LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
