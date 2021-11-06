//================================================================
// Created by João Vasco on 30/10/2021.
//================================================================

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

const std::string rgbFile = "../D435i/data/rgbData.bin";
const std::string depthFile = "../D435i/data/depthData.bin";
const std::string depthRGBFile = "../D435i/data/depthRGBData.bin";

uint16_t dataVideo[2764800] = {0};
uint16_t dataDepthRGB[2764800] = {0};
uint16_t dataDepth[1843200] = {0};


// Data structure to store the info about the frames
// This data is stored at the start of the binary file and
// MUST be the same on the save and read code!
typedef struct
{
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int pixelBytes = 0;
    unsigned int strideBytes = 0;
    unsigned int dataSize = 0;
} frameInfo;


int main(int argc, char *argv[])
{
  frameInfo depthInfo;
  frameInfo depthRGBInfo;
  frameInfo videoInfo;

  /*
   * Opening the binary Files
   */

  std::fstream rgb(rgbFile, std::ios::in | std::ios::binary);
  std::fstream depth(depthFile, std::ios::in | std::ios::binary);
  std::fstream depthRGB(depthRGBFile, std::ios::in | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open() || !depthRGB.is_open())
  {
    std::cout << "Failed while opening rgb or depth files." << std::endl;
    rgb.close();
    depth.close();
    return -1;
  }

  /*
   * Print some information about the data on the files
   */

  depth.read(reinterpret_cast<char *>(&depthInfo), sizeof(depthInfo));
  depthRGB.read(reinterpret_cast<char *>(&depthRGBInfo), sizeof(depthRGBInfo));
  rgb.read(reinterpret_cast<char *>(&videoInfo), sizeof(videoInfo));


  std::cout << "Depth Info Data" << std::endl << "====================" << std::endl;
  std::cout << "Data Size: " << depthInfo.dataSize << std::endl
            << "Stride: " << depthInfo.strideBytes << std::endl
            << "Height: " << depthInfo.height << std::endl
            << "Width: " << depthInfo.width << std::endl
            << "Bytes per pixel: " << depthInfo.pixelBytes << std::endl;
  std::cout << "====================" << std::endl;

  std::cout << "Depth RGB Info Data" << std::endl << "====================" << std::endl;
  std::cout << "Data Size: " << depthRGBInfo.dataSize << std::endl
            << "Stride: " << depthRGBInfo.strideBytes << std::endl
            << "Height: " << depthRGBInfo.height << std::endl
            << "Width: " << depthRGBInfo.width << std::endl
            << "Bytes per pixel: " << depthRGBInfo.pixelBytes << std::endl;
  std::cout << "====================" << std::endl;

  std::cout << "Video Info Data" << std::endl << "====================" << std::endl;
  std::cout << "Data Size: " << videoInfo.dataSize << std::endl
            << "Stride: " << videoInfo.strideBytes << std::endl
            << "Height: " << videoInfo.height << std::endl
            << "Width: " << videoInfo.width << std::endl
            << "Bytes per pixel: " << videoInfo.pixelBytes << std::endl;
  std::cout << "====================" << std::endl;

  /*
   * Do something with the data, in this case writing the first 10 frame to a .png
   * Can be replaced with while loop where the condition is:
   * rgb.read(reinterpret_cast<char *>(dataVideo), videoInfo.dataSize)
   * to read all the frames until the end of the file.
   */

  for (int i = 0; i < 10; ++i)
  {
    rgb.read(reinterpret_cast<char *>(dataVideo), videoInfo.dataSize);
    stbi_write_png(("Teste Video - " + std::to_string(i) + ".png").c_str(), videoInfo.width, videoInfo.height,
                   videoInfo.pixelBytes,
                   dataVideo, videoInfo.strideBytes);

    std::cout << "Ficheiro RGB Escrito! - " << i << std::endl;

    depth.read(reinterpret_cast<char *>(dataDepth), depthInfo.dataSize);
    /*
     * Do something with the raw depth data like aplying a custom color map or idk
     */

    depthRGB.read(reinterpret_cast<char *>(dataDepthRGB), depthRGBInfo.dataSize);
    stbi_write_png(("Teste DepthRGB - " + std::to_string(i) + ".png").c_str(), depthRGBInfo.width, depthRGBInfo.height,
                   depthRGBInfo.pixelBytes, dataDepthRGB, depthRGBInfo.strideBytes);

    std::cout << "Ficheiro Depth RGB Escrito! - " << i << std::endl;

  }

  return 0;
}
