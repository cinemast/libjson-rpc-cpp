file(GLOB mongoose_source *.c)
file(GLOB mongoose_header *.h)

include_directories(include/mongoose)

add_library(mongoose SHARED ${mongoose_source})
add_library(mongooseStatic STATIC ${mongoose_source})

set_target_properties(mongooseStatic PROPERTIES OUTPUT_NAME mongoose)


if(UNIX)
    target_link_libraries(mongoose dl pthread)
    target_link_libraries(mongooseStatic dl pthread)
endif(UNIX)

if(WIN32)
    target_link_libraries(mongoose Ws2_32)
    target_link_libraries(mongooseStatic Ws2_32)

endif(WIN32)

install(FILES ${mongoose_header} DESTINATION include/mongoose) 
install(TARGETS mongoose mongooseStatic LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
