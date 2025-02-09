#ifndef BMP_IMAGE_H // BMP_IMAGE_H
#define BMP_IMAGE_H

#include <cstdint>
#include <string>
#include <vector>

#pragma pack(push, 1)
struct BMPFileHeader {
  uint16_t bfType;
  uint32_t bfileSize;
  uint32_t bfReserved;
  uint32_t bOffBits;
};

struct DIBHeader {
  uint32_t biSize;
  int32_t biWidth;
  int32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
};
#pragma pack(pop)

struct Pixel {
  uint8_t b, g, r;
};

class BMPImage {
public:
  BMPImage() = default;

  static BMPImage load(const std::string &filename);

  void save(const std::string &filename) const;

  void flipHorizontal();
  void flipVertical();
  void rotateCW90();
  void rotateCCW90();

  int getWidth() const { return dibHeader.biWidth; }
  int getHeight() const { return dibHeader.biHeight; }
  int getBitCount() const { return dibHeader.biBitCount; }

private:
  BMPFileHeader fileHeader;
  DIBHeader dibHeader;
  std::vector<Pixel> data;

  int rowSize() const;
  int paddingSize() const;
};

#endif // BMP_IMAGE_H
