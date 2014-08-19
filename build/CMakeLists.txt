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

#add_definitions("-fsanitize-address-zero-base-shadow")
#add_definitions("-fsanitize=address")

add_definitions(-DUSE_WEBSOCKET)

if (UNIX AND NOT APPLE)
  add_definitions("-std=c++0x")
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
  ${sd}/ice/Utils.cpp
  ${sd}/ice/Candidate.cpp
  ${sd}/ice/Agent.cpp
  ${sd}/ice/Stream.cpp
  ${sd}/dtls/Context.cpp
  ${sd}/dtls/Parser.cpp
  ${sd}/rtc/Connection.cpp
  ${sd}/srtp/ParserSRTP.cpp
  ${sd}/rtp/ReaderVP8.cpp
  ${sd}/rtp/WriterVP8.cpp
  ${sd}/rtp/PacketVP8.cpp
  ${sd}/video/AggregatorVP8.cpp
  ${sd}/video/WriterIVF.cpp
  ${sd}/video/EncoderVP8.cpp
  ${sd}/video/DecoderVP8.cpp
  ${sd}/video/EncoderSettings.cpp

  # signaling
  ${sd}/signaling/Signaling.cpp
  ${sd}/signaling/Room.cpp
  ${sd}/signaling/SignalingSettings.cpp
  ${extern_source_dir}/mongoose.c
  ${extern_source_dir}/net_skeleton.c
  ${extern_source_dir}/ssl_wrapper.c
)

set(lib_headers
  ${id}
)

set(app_libs
  ${extern_lib_dir}/libssl.a                           # for hmac/sha/ssl
  ${extern_lib_dir}/libcrypto.a                        # for hmac/sha/ssl
  ${extern_lib_dir}/libuv.a                            # for networking
  ${extern_lib_dir}/libz.a                             # for crc32
  ${extern_lib_dir}/libsrtp.a                          # used to handle SRTP packets.
  ${extern_lib_dir}/libvpx.a                           # encoding/decoding vp8
  ${extern_lib_dir}/libvideogenerator.a                # used for test purposes; 
)

if (UNIX AND NOT APPLE)
  list(APPEND app_libs
    pthread
    dl
    )
endif()

set(webrtc_sources ${lib_sources})
set(webrtc_libs ${app_libs})

macro(create_test name)
  add_executable("test_webrtc_${name}${debug}" ${sd}/test_webrtc_${name}.cpp)
  target_link_libraries("test_webrtc_${name}${debug}" ${lib} ${app_libs})
  install(TARGETS "test_webrtc_${name}${debug}" DESTINATION bin)
endmacro()

add_library(${lib} STATIC ${lib_sources})
install(TARGETS ${lib} ARCHIVE DESTINATION lib)
install(DIRECTORY ${lib_headers} DESTINATION include)

# temporary disable the webrtc tests
create_test(ice)
create_test(ssl_fingerprint)
create_test(hmac_sha1)
create_test(stun_message_integrity)
create_test(zlib_crc32)
create_test(stun_message_fingerprint)
create_test(openssl_load_key_and_cert)
create_test(ice_agent)
create_test(dtls)
create_test(extract_keying_info_for_srtp)
create_test(video_encoder)
create_test(mongoose)
create_test(signaling)
