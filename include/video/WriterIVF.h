#ifndef VIDEO_WRITER_IVF
#define VIDEO_WRITER_IVF

#include <stdint.h>
#include <fstream>
#include <string>

namespace video {

  class WriterIVF {

  public:
    WriterIVF();
    ~WriterIVF();
    int open(std::string filepath, uint16_t width, uint16_t height, uint32_t fpsnum, uint32_t fpsden);
    int write(uint8_t* data, uint32_t nbytes, uint64_t timestamp); /* timestamp is actually the frame number ^.^ */
    int close();
  private:
    void writeU16(uint16_t v);
    void writeU32(uint32_t v);
    void writeU64(uint64_t v);

  public:
    std::ofstream ofs;
    uint64_t nframes;
  };

} /* namespace video */


#endif
