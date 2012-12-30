file(GLOB mongoose_source *.c)

include_directories(include/mongoose)

add_library(mongoose SHARED ${mongoose_source})
