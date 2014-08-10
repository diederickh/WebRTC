#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <video/EncoderSettings.h>
#include <video/EncoderVP8.h>
#include <rtp/WriterVP8.h>
#include <rtp/ReaderVP8.h>

extern "C" {
#  include <video_generator.h>
}

#define WIDTH 640
#define HEIGHT 480
#define FRAMERATE 25
#define RTP_PACKET_SIZE (1000)

video::EncoderSettings settings;
video::EncoderVP8 encoder;
rtp::WriterVP8 rtp_writer;

static void on_vp8_packet(video::EncoderVP8* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts);
static void on_rtp_packet(rtp::PacketVP8* pkt, void* user);

int main() {

  printf("\n\ntest_video_encoder\n\n");

  settings.width = WIDTH;
  settings.height = HEIGHT;
  settings.fps_num = 1;
  settings.fps_den = FRAMERATE;

  /* initialize the encoder. */
  if (encoder.init(settings) < 0) {
    printf("main - error: cannot initialize the vp8 encoder.\n");
    exit(1);
  }

  /* initialize the video generator. */
  video_generator gen;
  if (video_generator_init(&gen, WIDTH, HEIGHT, FRAMERATE) < 0) {
    printf("main - error: cannot initialize the video generator.\n");
    exit(1);
  }

  rtp_writer.on_packet = on_rtp_packet;
  encoder.on_packet = on_vp8_packet;

  while (1) {

    video_generator_update(&gen);
    if (gen.frame > 250) {
      break;
    }

    encoder.encode(gen.y, gen.strides[0],
                   gen.u, gen.strides[1],
                   gen.v, gen.strides[2], 
                   gen.frame);

    printf("-\n");

    usleep(gen.fps);
  }

  video_generator_clear(&gen);

  return 0;
}


static void on_vp8_packet(video::EncoderVP8* enc, const vpx_codec_cx_pkt_t* pkt, int64_t pts) {
  printf("on_vp8_packet - verbose: received a new packet from the encoder, pts: %lld\n", pts);
 

  /*
 uint8_t buffer[RTP_PACKET_SIZE];
  rtp::PacketVP8 rtp;
  rtp.version = 2;
  rtp.payload_type = 100;
  rtp.timestamp = pkt->data.frame.pts * 90;

  rtp::rtp_vp8_encode(pkt, &rtp, buffer, RTP_PACKET_SIZE);
  */

  rtp_writer.packetize(pkt);
  
  //rtp::rtp_vp8_encode(pkt, RTP_PACKET_SIZE, buffer, RTP_PACKET_SIZE);
}

static void on_rtp_packet(rtp::PacketVP8* pkt, void* user) {
  printf("on_rtp_packet - verbose, received rtp packet.\n");
  
  rtp::PacketVP8 output;
  rtp::rtp_vp8_decode(pkt->payload, pkt->nbytes, &output);
  
}
 
