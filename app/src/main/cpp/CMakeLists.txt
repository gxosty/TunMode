cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

if ((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"))
	add_definitions(-DRELEASE_MODE=1)
endif()

set(CMAKE_CXX_STANDARD 20)

project(TunMode)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/internal)

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
	src/tunmode/common/inbuffer.cxx
	src/tunmode/common/buffer.cxx
	src/tunmode/common/packet.cxx
	src/tunmode/common/utils.cxx
	src/tunmode/session/session.cxx
	src/tunmode/session/tcpsession.cxx
	src/tunmode/manager/sessionmanager.cxx
	src/tunmode/manager/tcpmanager.cxx
	src/tunmode/socket/socket.cxx
	src/tunmode/socket/sessionsocket.cxx
	src/tunmode/socket/tunsocket.cxx
	src/tunmode/socket/tcpsocket.cxx

	src/RawSocket/CheckSum.cpp
)

target_link_libraries(tunmode
	log
	android
	z
)