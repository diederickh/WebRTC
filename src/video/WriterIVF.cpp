#include <video/WriterIVF.h>

namespace video {
  
  WriterIVF::WriterIVF()
    :nframes(0)
  {
  }
  
  WriterIVF::~WriterIVF() {
    close();
  }
  
  int WriterIVF::open(std::string filepath, uint16_t width, uint16_t height, uint32_t fpsnum, uint32_t fpsden) {

    /* open output file */
    ofs.open(filepath.c_str(), std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
      printf("WriterIVF::init - error while trying to open the output file: `%s`\n", filepath.c_str());
      return -1;
    }

    /* write header */
    ofs.write("D", 1);
    ofs.write("K", 1);
    ofs.write("I", 1);
    ofs.write("F", 1);

    writeU16(0);               /* version, 0 */ 
    writeU16(32);              /* header size */ 
    writeU32(0x30385056);      /* four cc */
    writeU16(width);           /* width */
    writeU16(height);          /* height */
    writeU32(fpsden);          /* framerate denumerator */ 
    writeU32(fpsnum);          /* framerate numerator */
    writeU64(0);               /* frame count */
    
    return 0;
  }

  int WriterIVF::write(uint8_t* data, uint32_t nbytes, uint64_t timestamp) {
    if (!data) { return -1; } 
    if (!nbytes) { return -2; } 
    if (!ofs.is_open()) { return -3; } 

    printf("WriterIVF - verbose: writing %u bytes, %llu timestamp\n", nbytes, timestamp);
    writeU32(nbytes);
    writeU64(timestamp);
    ofs.write((const char*)data, nbytes);

    nframes++;
    
    return 0;
  }

  int WriterIVF::close() {
    if (ofs.is_open() && nframes > 0) {
      ofs.seekp(24);
      writeU64(nframes);
    }

    if (ofs.is_open()) {
      ofs.close();
    }

    return 0;
  }

  /* write little endian 16 */
  void WriterIVF::writeU16(uint16_t v) {
    const char* p = (const char*)&v;
    ofs.write(p + 0, 1);
    ofs.write(p + 1, 1);

  }

  /* write little endian 32 */
  void WriterIVF::writeU32(uint32_t v) {
    const char* p = (const char*)&v;
    ofs.write(p + 0, 1);
    ofs.write(p + 1, 1);
    ofs.write(p + 2, 1);
    ofs.write(p + 3, 1);
  }

  /* write little endian 64 */
  void WriterIVF::writeU64(uint64_t v)  {
    const char* p = (const char*)&v;
    ofs.write(p + 0, 1);
    ofs.write(p + 1, 1);
    ofs.write(p + 2, 1);
    ofs.write(p + 3, 1);
    ofs.write(p + 4, 1);
    ofs.write(p + 5, 1);
    ofs.write(p + 6, 1);
    ofs.write(p + 7, 1);
  }

} /* namespace video */
