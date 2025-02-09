#include "../include/bmp_image.h"

#include <fstream>
#include <stdexcept>
#include <vector>

BMPImage BMPImage::load(const std::string &filename) {
  BMPImage image;
  std::ifstream in(filename, std::ios::binary);
  if (!in) {
    throw std::runtime_error("Ошибка: не удалось открыть файл для чтения: " +
                             filename);
  }

  in.read(reinterpret_cast<char *>(&image.fileHeader), sizeof(BMPFileHeader));
  if (in.gcount() != sizeof(BMPFileHeader)) {
    throw std::runtime_error("Ошибка: не удалось прочитать BMP заголовок.");
  }
  if (image.fileHeader.bfType != 0x4D42) {
    throw std::runtime_error("Ошибка: файл не является BMP.");
  }

  in.read(reinterpret_cast<char *>(&image.dibHeader), sizeof(DIBHeader));
  if (in.gcount() != sizeof(DIBHeader)) {
    throw std::runtime_error("Ошибка: не удалось прочитать DIB заголовок.");
  }

  if (image.dibHeader.biBitCount != 24) {
    throw std::runtime_error(
        "Ошибка: поддерживаются только 24-битные BMP файлы.");
  }
  if (image.dibHeader.biCompression != 0) {
    throw std::runtime_error(
        "Ошибка: поддерживаются только несжатые BMP файлы.");
  }

  int width = image.dibHeader.biWidth;
  int height = image.dibHeader.biHeight;
  if (width <= 0 || height <= 0) {
    throw std::runtime_error("Ошибка: некорректные размеры изображения.");
  }

  int bytesPerPixel = image.dibHeader.biBitCount / 8;
  int rowSizeBytes = width * bytesPerPixel;
  int padSize = (4 - (rowSizeBytes % 4)) % 4;

  image.data.resize(width * height);

  in.seekg(image.fileHeader.bOffBits, std::ios::beg);
  if (!in) {
    throw std::runtime_error(
        "Ошибка: не удалось перейти к данным изображения.");
  }

  std::vector<char> rowBuffer(rowSizeBytes + padSize);
  for (int y = height - 1; y >= 0; --y) {
    in.read(rowBuffer.data(), rowSizeBytes + padSize);
    if (in.gcount() != static_cast<std::streamsize>(rowSizeBytes + padSize)) {
      throw std::runtime_error(
          "Ошибка: не удалось прочитать данные изображения.");
    }
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;
      image.data[idx].b = static_cast<uint8_t>(rowBuffer[x * bytesPerPixel]);
      image.data[idx].g =
          static_cast<uint8_t>(rowBuffer[x * bytesPerPixel + 1]);
      image.data[idx].r =
          static_cast<uint8_t>(rowBuffer[x * bytesPerPixel + 2]);
    }
  }

  return image;
}

void BMPImage::save(const std::string &filename) const {
  std::ofstream out(filename, std::ios::binary);
  if (!out) {
    throw std::runtime_error("Ошибка: не удалось открыть файл для записи: " +
                             filename);
  }

  int width = dibHeader.biWidth;
  int height = dibHeader.biHeight;
  int bytesPerPixel = dibHeader.biBitCount / 8;
  int rowSizeBytes = width * bytesPerPixel;
  int padSize = (4 - (rowSizeBytes % 4)) % 4;
  uint32_t imageSize = (rowSizeBytes + padSize) * height;

  BMPFileHeader header = fileHeader;
  DIBHeader dib = dibHeader;
  dib.biSizeImage = imageSize;
  header.bfileSize = header.bOffBits + imageSize;

  out.write(reinterpret_cast<const char *>(&header), sizeof(BMPFileHeader));
  out.write(reinterpret_cast<const char *>(&dib), sizeof(DIBHeader));

  std::vector<char> rowBuffer(rowSizeBytes + padSize, 0);

  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      int idx = y * width + x;
      rowBuffer[x * bytesPerPixel] = static_cast<char>(data[idx].b);
      rowBuffer[x * bytesPerPixel + 1] = static_cast<char>(data[idx].g);
      rowBuffer[x * bytesPerPixel + 2] = static_cast<char>(data[idx].r);
    }
    out.write(rowBuffer.data(), rowSizeBytes + padSize);
  }
}

int BMPImage::rowSize() const {
  int width = dibHeader.biWidth;
  int bytesPerPixel = dibHeader.biBitCount / 8;
  return width * bytesPerPixel;
}

int BMPImage::paddingSize() const {
  int rSize = rowSize();
  return (4 - (rSize % 4)) % 4;
}

void BMPImage::flipHorizontal() {
  int width = dibHeader.biWidth;
  int height = dibHeader.biHeight;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width / 2; ++x) {
      std::swap(data[y * width + x], data[y * width + (width - 1 - x)]);
    }
  }
}

void BMPImage::flipVertical() {
  int width = dibHeader.biWidth;
  int height = dibHeader.biHeight;
  for (int y = 0; y < height / 2; ++y) {
    for (int x = 0; x < width; ++x) {
      std::swap(data[y * width + x], data[(height - 1 - y) * width + x]);
    }
  }
}

void BMPImage::rotateCW90() {
  int oldWidth = dibHeader.biWidth;
  int oldHeight = dibHeader.biHeight;
  int newWidth = oldHeight;
  int newHeight = oldWidth;
  std::vector<Pixel> rotated(newWidth * newHeight);

  for (int ny = 0; ny < newHeight; ++ny) {
    for (int nx = 0; nx < newWidth; ++nx) {
      int old_x = oldWidth - 1 - ny;
      int old_y = nx;
      rotated[ny * newWidth + nx] = data[old_y * oldWidth + old_x];
    }
  }
  data = std::move(rotated);
  dibHeader.biWidth = newWidth;
  dibHeader.biHeight = newHeight;
}

void BMPImage::rotateCCW90() {
  int oldWidth = dibHeader.biWidth;
  int oldHeight = dibHeader.biHeight;
  int newWidth = oldHeight;
  int newHeight = oldWidth;
  std::vector<Pixel> rotated(newWidth * newHeight);

  for (int ny = 0; ny < newHeight; ++ny) {
    for (int nx = 0; nx < newWidth; ++nx) {
      int old_x = ny;
      int old_y = oldHeight - 1 - nx;
      rotated[ny * newWidth + nx] = data[old_y * oldWidth + old_x];
    }
  }
  data = std::move(rotated);
  dibHeader.biWidth = newWidth;
  dibHeader.biHeight = newHeight;
}
