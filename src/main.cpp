#include "../include/bmp_image.h"
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "Использование: " << argv[0]
              << " <входной BMP файл> <выходной BMP файл>\n";
    return 1;
  }

  try {
    BMPImage image = BMPImage::load(argv[1]);
    std::cout << "Изображение успешно загружено.\n";

    std::string query = argv[2];

    if (query == "none") {
    } else if (query == "cw90") {
      image.rotateCW90();
    } else if (query == "ccw90") {
      image.rotateCCW90();
    } else if (query == "fliph") {
      image.flipHorizontal();
    } else if (query == "flipv") {
      image.flipVertical();
    }

    image.save(argv[2]);
    std::cout << "Изображение успешно сохранено.\n";
  } catch (const std::exception &ex) {
    std::cerr << "Ошибка: " << ex.what() << "\n";
    return 1;
  }

  return 0;
}
