cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
project (atoc CXX)

# OS (Windows | UNIX)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set (WINDOWS ON)
else (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set (WINDOWS OFF)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set (CMAKE_CXX_STANDARD 14)

set(USE_ZLIB ON CACHE BOOL "Whether to enable support for zlib compression")
set(ZLIB_DIRECTORY "" CACHE PATH "Settable additionally to the ZLIB_ROOT environment variable")

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

file (GLOB atoc_SRC
	"src/*.cpp"
)
if (WINDOWS)
	set (atoc_SRC ${atoc_SRC} info.rc)
endif (WINDOWS)

set (CMAKE_CXX_FLAGS_DEBUG "-g")

# Libs
# -> Boost 
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

find_package (Boost REQUIRED COMPONENTS log filesystem program_options)
list (APPEND INCS ${Boost_INCLUDE_DIRS})
list (APPEND LIBS ${Boost_LIBRARIES})

#-> ZLIB
if(USE_ZLIB)
	# Environment variable for Windows
	set (ZLIB_ROOT $ENV{ZLIB_ROOT} ${ZLIB_DIRECTORY})
	# Use slightly modified FindZLIB with static linkage support
	set (ZLIB_STATIC ON)
	find_package (ZLIB_ REQUIRED)
	#list (APPEND INCS ${ZLIB_INCLUDE_DIRS})
	list (APPEND INCS ${ZLIB_INCLUDE_DIRS})
	list (APPEND LIBS ${ZLIB_LIBRARIES})
	add_definitions(-DCOMPRESS_ZIP)
endif(USE_ZLIB)

# Executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
add_executable (atoc ${atoc_SRC})

# Linking
include_directories (${INCS})
target_link_libraries (atoc ${LIBS})
