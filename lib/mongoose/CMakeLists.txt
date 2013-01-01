file(GLOB mongoose_source *.c)
file(GLOB mongoose_header *.h)

include_directories(include/mongoose)

add_library(mongoose SHARED ${mongoose_source})

target_link_libraries(mongoose dl pthread)

install(FILES ${mongoose_header} DESTINATION include/mongoose) 
install(TARGETS mongoose LIBRARY DESTINATION lib)
