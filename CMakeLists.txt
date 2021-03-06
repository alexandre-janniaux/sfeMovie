
cmake_minimum_required (VERSION 2.8)


# set a default build type if none was provided
# this has to be done before the project() instruction!
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

project (sfeMovie)

# detect the OS
if(${CMAKE_HOST_WIN32})
    set(WINDOWS 1)
elseif(${CMAKE_HOST_APPLE})
    set(MACOSX 1)
elseif(${CMAKE_HOST_UNIX})
    set(LINUX 1)
else()
    message(FATAL_ERROR "Unsupported operating system")
    return()
endif()


# Set some env vars
set (LIB_NAME sfe-movie)
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_PATCH 0)

set (SOURCES_DIR "src")
set (PUBLIC_HEADERS "include")
set (PRIVATE_HEADERS "src")
set (EXTERNAL_HEADERS "deps/headers" "deps/SFML/include")
set (FFMPEG_LIBRARIES avdevice avformat avcodec avutil swscale)
set (OTHER_LIBRARIES "")
set (SFML_LIBRARIES sfml-system sfml-audio sfml-window sfml-graphics)
set (EXTERNAL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/deps/ffmpeg-build ${CMAKE_SOURCE_DIR}/deps/SFML/lib)
set (COMPILER_FLAGS "")
set (LINKER_FLAGS "")

add_definitions("-O2")
set (ARCH "x86_64")
set (SOURCE_FILES ${SOURCES_DIR}/Movie.cpp ${SOURCES_DIR}/Movie_audio.cpp ${SOURCES_DIR}/Movie_video.cpp ${SOURCES_DIR}/utils.cpp ${SOURCES_DIR}/Condition.cpp)


if (LINUX) # ========================================== LINUX ========================================== #


	set (EXTERNAL_LIBRARIES_DIR ${EXTERNAL_LIBRARIES_DIR} "${CMAKE_SOURCE_DIR}/deps/linux-binaries" "${CMAKE_SOURCE_DIR}/deps/ffmpeg-build")
	set (SOURCE_FILES ${SOURCE_FILES} "${SOURCES_DIR}/Unix/ConditionImpl.cpp")
	add_definitions(-D__STDC_CONSTANT_MACROS)
	set (COMPILER_FLAGS ${COMPILER_FLAGS} "-O2")
	set (SFML_LIBRARIES ${SFML_LIBRARIES} "sfml-system" "sfml-window" "sfml-graphics" "sfml-audio")
	
	
elseif (MACOSX) # ========================================== MACOSX ========================================== #

	if (HAS_VDA)
		set (OTHER_LIBRARIES ${OTHER_LIBRARIES} z bz2 "-framework VideoDecodeAcceleration" "-framework CoreVideo")
	else()
		set (OTHER_LIBRARIES ${OTHER_LIBRARIES} z bz2)
	endif()
	
	# set (COMPILER_FLAGS ${COMPILER_FLAGS} "-O2")
	set (SOURCE_FILES ${SOURCE_FILES} "${SOURCES_DIR}/Unix/ConditionImpl.cpp")
	
	set (CMAKE_OSX_ARCHITECTURES ${ARCH})
    
    # add an option to let the user specify a custom directory for frameworks installation (SFML, sndfile, ...)
    set(CMAKE_INSTALL_FRAMEWORK_PREFIX "/Library/Frameworks" CACHE STRING "Frameworks installation directory")
	
	# find only "root" headers
    file(GLOB SFE_HEADERS RELATIVE ${PROJECT_SOURCE_DIR} "include/*")
    
    set (SOURCE_FILES ${SOURCE_FILES} ${SFE_HEADERS})
	
	
elseif (WINDOWS) # ========================================== WINDOWS ========================================== #


	set (EXTERNAL_LIBRARIES_DIR ${EXTERNAL_LIBRARIES_DIR} "${CMAKE_SOURCE_DIR}/deps/windows-binaries" "${CMAKE_SOURCE_DIR}/deps/ffmpeg-build")
	
	if (MSVC)
		set (EXTERNAL_LIBRARIES_DIR ${EXTERNAL_LIBRARIES_DIR} "${CMAKE_SOURCE_DIR}/deps/windows-binaries/msvc")
		set (PRIVATE_HEADERS ${PRIVATE_HEADERS} "${CMAKE_SOURCE_DIR}/deps/headers/msvc")
		set (FFMPEG_LIBRARIES "deps/ffmpeg-build/avdevice" "deps/ffmpeg-build/avformat" "deps/ffmpeg-build/avcodec" "deps/ffmpeg-build/avutil" "deps/ffmpeg-build/swscale")
	else()
		set (EXTERNAL_LIBRARIES_DIR ${EXTERNAL_LIBRARIES_DIR} "${CMAKE_SOURCE_DIR}/deps/windows-binaries/gcc")
		set (LINKER_FLAGS ${LINKER_FLAGS} "-static-libgcc")
		set (OTHER_LIBRARIES ${OTHER_LIBRARIES} "z")
		set (COMPILER_FLAGS ${COMPILER_FLAGS} "-O2")
	endif()
	
	add_definitions(-D__STDC_CONSTANT_MACROS -DSFE_EXPORTS)
	set (SFML_LIBRARIES ${SFML_LIBRARIES} "sfml-system" "sfml-window" "sfml-graphics" "sfml-audio")
	set (OTHER_LIBRARIES ${OTHER_LIBRARIES} "ws2_32")
	set (SOURCE_FILES ${SOURCE_FILES} "${SOURCES_DIR}/Win32/ConditionImpl.cpp")
endif()


# Build up to date SFML binaries 
if (MACOSX)
	set(BUILD_FRAMEWORKS 1) 
endif()
add_subdirectory("deps/SFML")


# Set the search directories for compiling and link editing
include_directories (${PUBLIC_HEADERS} ${PRIVATE_HEADERS} ${EXTERNAL_HEADERS})
link_directories (${EXTERNAL_LIBRARIES_DIR})


# Define the source files sfe should compile
add_library (${LIB_NAME} SHARED ${SOURCE_FILES})


set_target_properties(${LIB_NAME} PROPERTIES
					  COMPILE_FLAGS "${COMPILER_FLAGS}"
					  LINK_FLAGS "${LINKER_FLAGS}"
					  SOVERSION "${VERSION_MAJOR}.${VERSION_MINOR}.0"
                      VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")


if (MACOSX)
    # edit target properties
    set_target_properties(${LIB_NAME} PROPERTIES 
                          FRAMEWORK TRUE
                          FRAMEWORK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                          MACOSX_FRAMEWORK_IDENTIFIER org.yalir.${LIB_NAME}
                          MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                          MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                          PUBLIC_HEADER ${SFE_HEADERS} )


    # adapt install directory to allow distributing dylibs/frameworks in user’s frameworks/application bundle
    set_target_properties(${LIB_NAME} PROPERTIES 
                          BUILD_WITH_INSTALL_RPATH 1 
                          INSTALL_NAME_DIR "@executable_path/../Frameworks")
    
    install(TARGETS ${LIB_NAME}
            FRAMEWORK DESTINATION ${CMAKE_INSTALL_FRAMEWORK_PREFIX}
            COMPONENT devel)
endif()


# Define the libraries sfe should link against
target_link_libraries (${LIB_NAME} ${SFML_LIBRARIES} ${FFMPEG_LIBRARIES} ${OTHER_LIBRARIES})

