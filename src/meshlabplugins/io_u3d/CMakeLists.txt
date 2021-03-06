# Copyright 2019-2020, Collabora, Ltd.
# SPDX-License-Identifier: BSL-1.0


# Only build if we have u3d
if(TARGET external-IDTFConverter)

	set(SOURCES io_u3d.cpp)

	set(HEADERS io_u3d.h)
	include_directories("${EXTERNAL_DIR}/u3d/src/IDTF")

	add_library(io_u3d MODULE ${SOURCES} ${HEADERS})

	target_include_directories(io_u3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	
	target_link_libraries(io_u3d PUBLIC meshlab-common)

	target_link_libraries(io_u3d PUBLIC external-IDTFConverter IFXCoreStatic ${CMAKE_DL_LIBS})

	set_property(TARGET io_u3d PROPERTY FOLDER Plugins)

	set_property(TARGET io_u3d PROPERTY RUNTIME_OUTPUT_DIRECTORY
										${MESHLAB_PLUGIN_OUTPUT_DIR})

	set_property(TARGET io_u3d PROPERTY LIBRARY_OUTPUT_DIRECTORY
										${MESHLAB_PLUGIN_OUTPUT_DIR})

	install(
		TARGETS io_u3d
		DESTINATION ${MESHLAB_PLUGIN_INSTALL_DIR}
		COMPONENT Plugins)

else()
	message(STATUS "Skipping io_u3d - missing u3d in external directory.")
endif()
