file(GLOB mongoose_source *.c)
file(GLOB mongoose_header *.h)

include_directories(include/mongoose)

add_library(mongoose SHARED ${mongoose_source})

if(UNIX)
    target_link_libraries(mongoose dl pthread)
endif(UNIX)

if(WIN32)
    target_link_libraries(mongoose Ws2_32)
endif(WIN32)

install(FILES ${mongoose_header} DESTINATION include/mongoose) 
install(TARGETS mongoose LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib
                      RUNTIME DESTINATION bin)
