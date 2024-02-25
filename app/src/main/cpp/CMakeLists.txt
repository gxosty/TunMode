cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Release)

if ((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))
	add_definitions(-DRELEASE_MODE=1)
endif()

set(CMAKE_CXX_STANDARD 20)

project(TunMode)

include_directories(${CMAKE_SOURCE_DIR}/include)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
	set(DPIMASK_LIBS_DIR "${CMAKE_SOURCE_DIR}/lib/armeabi-v7a")
elseif (${ANDROID_ABI} STREQUAL "arm64-v8a")
	set(DPIMASK_LIBS_DIR "${CMAKE_SOURCE_DIR}/lib/arm64-v8a")
else()
	message(SEND_ERROR "Wrong ANDROID_ABI: ${ANDROID_ABI}")
endif()

link_directories(${DPIMASK_LIBS_DIR})

# add_library(crypto SHARED IMPORTED)
# set_target_properties(
# 	crypto
# 	PROPERTIES IMPORTED_LOCATION
# 	${DPIMASK_LIBS_DIR}/libcrypto.so
# )

# add_library(ssl SHARED IMPORTED)
# set_target_properties(
# 	ssl
# 	PROPERTIES IMPORTED_LOCATION
# 	${DPIMASK_LIBS_DIR}/libssl.so
# )

# add_library(curl SHARED IMPORTED)
# set_target_properties(
# 	curl
# 	PROPERTIES IMPORTED_LOCATION
# 	${DPIMASK_LIBS_DIR}/libcurl.so
# )

add_library(tunmode SHARED
	main.cxx

	src/tunmode/tunmode.cxx
)

target_link_libraries(tunmode
	log
	android
	z
)