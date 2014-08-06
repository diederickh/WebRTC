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
  ${sd}/stun/Reader.cpp
  ${sd}/stun/Writer.cpp
  ${sd}/stun/Message.cpp
  ${sd}/stun/Attribute.cpp
  ${sd}/stun/Types.cpp
  ${sd}/stun/Utils.cpp
  ${sd}/ice/ICE.cpp
  ${sd}/ice/Utils.cpp
  ${sd}/ice/Candidate.cpp
  ${sd}/ice/Agent.cpp
  ${sd}/ice/Stream.cpp
  ${sd}/dtls/Context.cpp
  ${sd}/dtls/Parser.cpp
  ${sd}/rtc/Connection.cpp
)

set(lib_headers
  ${id}
)

set(app_libs
  ${extern_lib_dir}/libssl.a             # for hmac/sha/ssl
  ${extern_lib_dir}/libcrypto.a          # for hmac/sha/ssl
  ${extern_lib_dir}/libuv.a              # for networking
  ${extern_lib_dir}/libz.a               # for crc32
  ${extern_lib_dir}/libjansson.a         # for signaling, parsing json messages
  ${extern_lib_dir}/libwebsockets.a      # for signaling, handling i/o
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

set(test test_hmac_sha1)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_stun_message_integrity)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_zlib_crc32)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_stun_message_fingerprint)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_openssl_load_key_and_cert)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_ice_agent)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_libwebsockets)
add_executable("${test}${debug}" ${sd}/${test}.cpp)
set_target_properties("${test}${debug}" PROPERTIES COMPILE_FLAGS "-std=c++0x")
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)

set(test test_dtls)
add_executable("${test}${debug}" ${sd}/${test}.c)
target_link_libraries("${test}${debug}" ${lib} ${app_libs})
install(TARGETS "${test}${debug}" DESTINATION bin)