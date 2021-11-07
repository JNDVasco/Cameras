//================================================================
// Created by João Vasco on 30/10/2021.
//================================================================

#include <sl/Camera.hpp>

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams


const std::string rgbFile = "../ZED/data/rgbData.bin";
const std::string depthFile = "../ZED/data/depthData.bin";
const std::string depthRawFile = "../ZED/data/depthRawData.bin";


// Data structure to store the info about the frames
// This data is stored at the start of the binary file and
// MUST be the same on the save and read code!
typedef struct
{
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int pixelBytes = 0;
    unsigned int step = 0;
    unsigned int widthBytes = 0;
} frameInfo;

sl::uchar1 dataRGB[3686400] = {0};
sl::uchar1 dataDepth[3686400] = {0};
sl::uchar1 dataRawDepth[14745600] = {0};

int main(int argc, char *argv[])
{
  frameInfo depthInfo;
  frameInfo depthRawInfo;
  frameInfo rgbInfo;

  /*
   * Open the files and check if they are open
   */

  std::fstream rgb(rgbFile, std::ios::in | std::ios::binary);
  std::fstream depth(depthFile, std::ios::in | std::ios::binary);
  std::fstream depthRaw(depthRawFile, std::ios::in | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open() || !depthRaw.is_open())
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
  rgb.read(reinterpret_cast<char *>(&rgbInfo), sizeof(rgbInfo));
  depthRaw.read(reinterpret_cast<char *>(&depthRawInfo), sizeof(depthRawInfo));


  std::cout << "Depth Info Data" << std::endl << "====================" << std::endl;
  std::cout << "Data Size: " << depthInfo.widthBytes * depthInfo.height << std::endl
            << "Height: " << depthInfo.height << std::endl
            << "Width: " << depthInfo.width << std::endl
            << "Bytes per Pixel: " << depthInfo.pixelBytes << std::endl;
  std::cout << "====================" << std::endl;

  std::cout << "Video Info Data" << std::endl << "====================" << std::endl;
  std::cout << "Data Size: " << rgbInfo.widthBytes * rgbInfo.height << std::endl
            << "Height: " << rgbInfo.height << std::endl
            << "Width: " << rgbInfo.width << std::endl
            << "Bytes per Pixel: " << rgbInfo.pixelBytes << std::endl;
  std::cout << "====================" << std::endl;

  std::cout << "Raw Depth Info Data" << std::endl << "====================" << std::endl;
  std::cout << "Data Size: " << depthRawInfo.widthBytes * depthRawInfo.height << std::endl
            << "Height: " << depthRawInfo.height << std::endl
            << "Width: " << depthRawInfo.width << std::endl
            << "Bytes per Pixel: " << depthRawInfo.pixelBytes << std::endl;
  std::cout << "====================" << std::endl;


  sl::Mat rgbMatrix(rgbInfo.width, rgbInfo.height, sl::MAT_TYPE::U8_C4, dataRGB, rgbInfo.step);
  sl::Mat depthMatrix(depthInfo.width, depthInfo.height, sl::MAT_TYPE::U8_C4, dataDepth, depthInfo.step);
  sl::Mat depthRawMatrix(depthRawInfo.width, depthRawInfo.height, sl::MAT_TYPE::U8_C4, dataRawDepth, depthRawInfo.step);

  /*
   * Do something with the data, in this case writing the first 10 frame to a .png
   * Can be replaced with while loop where the condition is:
   * rgb.read(reinterpret_cast<char *>(dataRGB), (rgbInfo.widthBytes * rgbInfo.height))
   * to read all the frames until the end of the file.
   */

  for (int i = 0; i < 10; ++i)
  {
    sl::String filename1(("Teste RGB ZED2 - " + std::to_string(i) + ".png").c_str());
    sl::String filename2(("Teste Depth RGB ZED2 - " + std::to_string(i) + ".png").c_str());
    sl::String filename3(("Teste Depth RAW ZED2 - " + std::to_string(i) + ".wyz   ").c_str());


    rgb.read(reinterpret_cast<char *>(dataRGB), (rgbInfo.widthBytes * rgbInfo.height));

    rgbMatrix.write(filename1);

    std::cout << "Ficheiro RGB Escrito! - " << i << std::endl;

    depth.read(reinterpret_cast<char *>(dataDepth), (depthInfo.widthBytes * depthInfo.height));

    depthMatrix.write(filename2);

    std::cout << "Ficheiro Depth RGB Escrito! - " << i << std::endl;


/*
    depthRaw.read(reinterpret_cast<char *>(dataRawDepth), (depthRawInfo.widthBytes * depthRawInfo.height));

    depthRawMatrix.write(filename3);

    std::cout << "Ficheiro Depth Point Cloud Escrito! - " << i << std::endl;

    Descobrir o que é este erro:
    OpenCV Error: Unspecified error (could not find a writer for the specified extension) in slutils::cv::imwrite_, file C:\
    builds\sl\ZEDKit\lib\src\sl_core\opencv\imgcodecs\loadsave.cpp, line 466

*/


  }


  rgb.close();
  depth.close();
  depthRaw.close();

  return 0;
}
