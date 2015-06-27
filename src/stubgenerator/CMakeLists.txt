file(GLOB_RECURSE jsonrpcstub_source *.cpp)
file(GLOB_RECURSE jsonrpcstub_header *.h)
list(REMOVE_ITEM jsonrpcstub_source "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

include_directories(..)
include_directories(${JSONCPP_INCLUDE_DIRS})
include_directories(${ARGTABLE_INCLUDE_DIRS})

set(ALL_LIBS)


if (BUILD_SHARED_LIBS)
        add_library(libjsonrpcstub SHARED ${jsonrpcstub_source} ${jsonrpcstub_header} ${jsonrpc_helper_source_common})
        target_link_libraries(libjsonrpcstub jsonrpccommon ${JSONCPP_LIBRARIES} ${ARGTABLE_LIBRARIES})
        set_target_properties(libjsonrpcstub PROPERTIES OUTPUT_NAME jsonrpccpp-stub)
        list(APPEND ALL_LIBS libjsonrpcstub)
endif()

# setup static common library
if (BUILD_STATIC_LIBS)
        add_library(libjsonrpcstubStatic STATIC ${jsonrpcstub_source} ${jsonrpcstub_header} ${jsonrpc_helper_source_common})
        target_link_libraries(libjsonrpcstubStatic jsonrpccommonStatic ${JSONCPP_LIBRARIES} ${ARGTABLE_LIBRARIES})
        set_target_properties(libjsonrpcstubStatic PROPERTIES OUTPUT_NAME jsonrpccpp-stub)

        if (NOT BUILD_SHARED_LIBS)
                add_library(libjsonrpcstub ALIAS libjsonrpcstubStatic)
        endif()

        list(APPEND ALL_LIBS libjsonrpcstubStatic)
endif()

set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})
set_target_properties(
        ${ALL_LIBS}
        PROPERTIES VERSION "${VERSION_STRING}" SOVERSION "${SO_VERSION}"
)


add_executable(jsonrpcstub main.cpp)
target_link_libraries(jsonrpcstub jsonrpccommon libjsonrpcstub )

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

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake/libjsonrpccpp-stub.pc.cmake ${CMAKE_BINARY_DIR}/libjsonrpccpp-stub.pc)

INSTALL(FILES
    "${CMAKE_BINARY_DIR}/libjsonrpccpp-stub.pc"
    DESTINATION "lib/${CMAKE_LIBRARY_PATH}/pkgconfig")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/stubgenerator/
        DESTINATION include/jsonrpccpp/stubgen
        FILES_MATCHING PATTERN "*.h")

install(TARGETS jsonrpcstub ${ALL_LIBS} LIBRARY DESTINATION lib${LIB_SUFFIX}
	ARCHIVE DESTINATION lib${LIB_SUFFIX}
	RUNTIME DESTINATION bin
)

