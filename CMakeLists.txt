cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
project (atoc CXX)

# OS (Windows | UNIX)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set (WINDOWS ON)
else (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set (WINDOWS OFF)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set (CMAKE_CXX_STANDARD 14)

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
set (INCS ${INCS} ${Boost_INCLUDE_DIRS})
set (LIBS ${LIBS} ${Boost_LIBRARIES})

# Executable
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
add_executable (atoc ${atoc_SRC})

# Linking
include_directories (${INCS})
target_link_libraries (atoc ${LIBS})
