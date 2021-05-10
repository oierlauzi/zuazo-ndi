cmake_minimum_required(VERSION 3.9)
include(GNUInstallDirs)

#Project info
project(zuazo-ndi
	VERSION 0.1.0 
	DESCRIPTION "Compressed video IO for Zuazo"
)

#Subdirectories
#add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/doc/doxygen/)

#Set compiler's options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

#Register all source and header files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE INLINE_SOURCES ${PROJECT_SOURCE_DIR}/include/*.inl)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(
	${PROJECT_NAME} PROPERTIES 
		SOVERSION ${PROJECT_VERSION}	
)
#add_dependencies(${PROJECT_NAME} shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${PROJECT_NAME} PRIVATE ${SHADER_INCLUDE_DIR}/)

# Install library's binary files and headers
install(TARGETS ${PROJECT_NAME} 
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install NDI binaries
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
	set(NDI_LIB_DIR ${PROJECT_SOURCE_DIR}/lib/x86_64-linux-gnu)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86)
	set(NDI_LIB_DIR ${PROJECT_SOURCE_DIR}/lib/i686-linux-gnu)
else()
	message(SEND_ERROR "NDI binaries for arch ${CMAKE_SYSTEM_PROCESSOR} were not found")
endif()
set(NDI_LIB_BIN libndi.so.4.5.3)

install(FILES ${NDI_LIB_DIR}/${NDI_LIB_BIN}
		DESTINATION ${CMAKE_INSTALL_LIBDIR} )
