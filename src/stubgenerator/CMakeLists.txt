file(GLOB_RECURSE jsonrpcstub_source *.cpp)
file(GLOB_RECURSE libstubgen_source *.cpp)
list(REMOVE_ITEM libstubgen_source "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

include_directories(..)
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${ARGTABLE_INCLUDE_DIRS})

if (COMPILE_TESTS)
	add_library(stubgen STATIC ${libstubgen_source})
	target_link_libraries(stubgen jsonrpccommon ${ARGTABLE_LIBRARIES})
ENDIF()

add_executable(jsonrpcstub ${jsonrpcstub_source})
target_link_libraries(jsonrpcstub jsonrpccommon ${ARGTABLE_LIBRARIES})

#Generate manpage
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	configure_file("${CMAKE_SOURCE_DIR}/doc/manpage.in" "${CMAKE_BINARY_DIR}/manpage" @ONLY)
	add_custom_command(OUTPUT jsonrpcstub.1.gz
		COMMAND gzip -c "${CMAKE_BINARY_DIR}/manpage" > ${CMAKE_BINARY_DIR}/jsonrpcstub.1.gz
		COMMENT "Generating man pages"
		VERBATIM
	)

	add_custom_target(manpage ALL DEPENDS jsonrpcstub.1.gz)
	install(FILES ${CMAKE_BINARY_DIR}/jsonrpcstub.1.gz DESTINATION share/man/man1/)
	add_dependencies(jsonrpcstub manpage)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

install(TARGETS jsonrpcstub LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
)

