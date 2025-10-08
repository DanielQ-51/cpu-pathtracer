/*
Handles the image storage.

Uses a 1d vector of pixels instead of 2d for minor optimization.

*/

#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "object.h"
struct Vec3;
using Color = Vec3;

class Image 
{
public:
    Image(int w, int h);
    ~Image();

    void setColor(int x, int y, Color c);
    Color getColor(int x, int y);
    void saveImageBMP(std::string fileName);

private:
    int width, height;
    std::vector<Color> pixels;
    int toIndex(int x, int y);
};
