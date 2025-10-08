/*
Handles the image writing.

Uses a 1d vector of pixels instead of 2d for minor optimization.

*/

#include "image.h"
#include "bmp.h"
#include "object.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>

using Color = Vec3;

Image::Image(int w, int h) : width(w), height(h), pixels(std::vector<Color>(w * h)) {}

Image::~Image() {}

int Image::toIndex(int x, int y) {
    return y * width + x;
}

void Image::setColor(int x, int y, Color c) {
    pixels[toIndex(x, y)] = c;
}

Color Image::getColor(int x, int y) {
    return pixels[toIndex(x, y)];
}

void Image::saveImageBMP(std::string fileName) {
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    createBMPHeaders(width, height, fileHeader, infoHeader);

    std::ofstream out(fileName, std::ios::binary);
    out.write((char*)&fileHeader, sizeof(fileHeader));
    out.write((char*)&infoHeader, sizeof(infoHeader));

    int rowSize = (3 * width + 3) & (~3); // each row padded to multiple of 4 bytes
    int diff = width - rowSize;

    Color c;
    unsigned char row[rowSize];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            c = getColor(x, y);
            //c /= std::max(c[2], std::max(c[0], c[1]));

            //std::cout << c.b << " " << c.g << " " << c.r << std::endl; c[2]/max(c[2].x(),c[2].y(),c[2].z())

            row[x*3 + 0] = static_cast<unsigned char>(std::clamp(c[2], 0.0, 1.0) * 255.0f + 0.5f);
            row[x*3 + 1] = static_cast<unsigned char>(std::clamp(c[1], 0.0, 1.0) * 255.0f + 0.5f);
            row[x*3 + 2] = static_cast<unsigned char>(std::clamp(c[0], 0.0, 1.0) * 255.0f + 0.5f);

            //std::cout << static_cast<unsigned char>(c.b * 255.0f + 0.5f) << " " << static_cast<unsigned char>(c.g * 255.0f + 0.5f) << " " << static_cast<unsigned char>(c.r * 255.0f + 0.5f) << std::endl;
        }

        for (int i = 0; i < diff*3; i++) {
            row[width*3 + i] = 0;
        }
        out.write((char*)row, rowSize);
    }

    out.close();
}

void createBMPHeaders(int width, int height, BMPFileHeader &fileHeader, BMPInfoHeader &infoHeader) {
    int rowSize = (3 * width + 3) & (~3);
    int imageSize = rowSize * height;

    // File header
    fileHeader.bfType = 0x4D42;
    fileHeader.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    // Info header
    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = imageSize;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;
}