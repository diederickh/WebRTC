cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(app webrtc)
set(lib webrtc)
set(sd ${CMAKE_CURRENT_LIST_DIR}/../src/)
set(id ${CMAKE_CURRENT_LIST_DIR}/../include/)
set(debug "")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app "${app}_debug")
  set(debug "debug")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../extern/tinylib/src
)

set(lib_sources 
  ${sd}/sdp/SDP.cpp
  ${sd}/sdp/Utils.cpp
  ${sd}/sdp/Types.cpp
  ${sd}/sdp/Reader.cpp
  ${sd}/sdp/Writer.cpp
  ${sd}/ice/ICE.cpp
  ${sd}/ice/STUN.cpp
  ${sd}/ice/Utils.cpp
  ${sd}/rtc/DTLS.cpp
  ${sd}/rtc/Connection.cpp
)

set(lib_headers
  ${id}
)

set(app_libs
  ${extern_lib_dir}/libssl.a
  ${extern_lib_dir}/libcrypto.a
  ${extern_lib_dir}/libuv.a
)

add_library(${lib} STATIC ${lib_sources})
install(TARGETS ${lib} ARCHIVE DESTINATION lib)
install(DIRECTORY ${lib_headers} DESTINATION include)

set(test test_ice)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_ssl_fingerprint)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)