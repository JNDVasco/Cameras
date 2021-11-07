//================================================================
// Created by João Vasco on 15/10/2021.
//================================================================

#include <sl/Camera.hpp>
#include <fstream>              // File IO
#include <iostream>             // Terminal IO

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


int main(int argc, char **argv)
{
  frameInfo depthInfo;
  frameInfo depthRawInfo;
  frameInfo rgbInfo;

  /*
   * Open the files and check if they are open
   */

  std::fstream rgb(rgbFile, std::ios::out | std::ios::binary);
  std::fstream depth(depthFile, std::ios::out | std::ios::binary);
  std::fstream depthRaw(depthRawFile, std::ios::out | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open() || !depthRaw.is_open())
  {
    std::cout << "Failed while opening rgb or depth files." << std::endl;
    rgb.close();
    depth.close();
    return -1;
  }

  /*
   * Create a camera object
   */
  sl::Camera zed;

  /*
   * Set configuration parameters for the ZED
   * In this case we set
   *  - Resolution = 1280x720
   *  - Fps 30
   *  - depth data to "Quality"
   */

  sl::InitParameters init_parameters;
  init_parameters.camera_resolution = sl::RESOLUTION::HD720;
  init_parameters.camera_fps = 30;
  init_parameters.depth_mode = sl::DEPTH_MODE::QUALITY;

  /*
   * Open the camera
   */
  auto returned_state = zed.open(init_parameters);

  if (returned_state != sl::ERROR_CODE::SUCCESS)
  {
    std::cout << "Error " << returned_state << ", exit program." << std::endl;
    return EXIT_FAILURE;
  }

  /*
   * Get the resolution of the camera
   */
  sl::Resolution imageSize = zed.getCameraInformation().camera_configuration.resolution;


  /*
   * Create a matrix that holds the RGB data and print some information about it
   */
  sl::Mat rgbData(imageSize, sl::MAT_TYPE::U8_C4);

  std::cout << "  W: " << rgbData.getWidth()
            << "  H: " << rgbData.getHeight()
            << "  Bytes per Row: " << rgbData.getWidthBytes()
            << "  Bytes per Pixel: " << rgbData.getPixelBytes()
            << "  Step: " << rgbData.getStepBytes()
            << "  Total Size: " << rgbData.getWidthBytes() * rgbData.getHeight() << std::endl;


  rgbInfo.width = rgbData.getWidth();
  rgbInfo.height = rgbData.getHeight();
  rgbInfo.pixelBytes = rgbData.getPixelBytes();
  rgbInfo.step = rgbData.getStepBytes();
  rgbInfo.widthBytes = rgbData.getWidthBytes();

  /*
   * Create a matrix that holds the depth data and print some information about it
   */

  sl::Mat depthData(imageSize, sl::MAT_TYPE::U8_C4);

  std::cout << "  W: " << depthData.getWidth()
            << "  H: " << depthData.getHeight()
            << "  Bytes per Row: " << depthData.getWidthBytes()
            << "  Bytes per Pixel: " << depthData.getPixelBytes()
            << "  Step: " << depthData.getStepBytes()
            << "  Total Size: " << depthData.getWidthBytes() * depthData.getHeight() << std::endl;

  depthInfo.width = depthData.getWidth();
  depthInfo.height = depthData.getHeight();
  depthInfo.pixelBytes = depthData.getPixelBytes();
  depthInfo.step = depthData.getStepBytes();
  depthInfo.widthBytes = depthData.getWidthBytes();


  /*
   * Create a matrix that holds the depth data and print some information about it
   */

  sl::Mat depthRawData(imageSize, sl::MAT_TYPE::F32_C4);

  std::cout << "  W: " << depthRawData.getWidth()
            << "  H: " << depthRawData.getHeight()
            << "  Bytes per Row: " << depthRawData.getWidthBytes()
            << "  Bytes per Pixel: " << depthRawData.getPixelBytes()
            << "  Step: " << depthRawData.getStepBytes()
            << "  Total Size: " << depthRawData.getWidthBytes() * depthRawData.getHeight() << std::endl;

  depthRawInfo.width = depthRawData.getWidth();
  depthRawInfo.height = depthRawData.getHeight();
  depthRawInfo.pixelBytes = depthRawData.getPixelBytes();
  depthRawInfo.step = depthRawData.getStepBytes();
  depthRawInfo.widthBytes = depthRawData.getWidthBytes();


  /*
   * Save the information of the frames
   */
  depth.write(reinterpret_cast<char *>(&depthInfo), sizeof(depthInfo));
  depthRaw.write(reinterpret_cast<char *>(&depthRawInfo), sizeof(depthRawInfo));
  rgb.write(reinterpret_cast<char *>(&rgbInfo), sizeof(rgbInfo));

  for (int i = 0; i < 10; ++i)
  {
    // Grab an image
    returned_state = zed.grab();
    // A new image is available if grab() returns ERROR_CODE::SUCCESS
    if (returned_state == sl::ERROR_CODE::SUCCESS)
    {
      std::cout << "Frames!" << std::endl;
      /*
       * Grab a rgb image from the ZED
       */
      zed.retrieveImage(rgbData, sl::VIEW::LEFT);
      rgb.write(reinterpret_cast<char *>(rgbData.getPtr<sl::uchar1>()),
                (rgbData.getWidthBytes() * rgbData.getHeight())); //Save to a file


      /*
       * Grab a depth image from the ZED
       */
      zed.retrieveImage(depthData, sl::VIEW::DEPTH);
      depth.write(reinterpret_cast<char *>(depthData.getPtr<sl::uchar1>()),
                  (depthData.getWidthBytes() * depthData.getHeight())); //Save to a file


      /*
       * Grab a raw depth image from the ZED
       */

      zed.retrieveMeasure(depthRawData, sl::MEASURE::XYZ);
      depthRaw.write(reinterpret_cast<char *>(depthRawData.getPtr<sl::uchar1>()),
                  (depthRawData.getWidthBytes() * depthRawData.getHeight())); //Save to a file

    }
  }

  /*
   * Close the camera and the files
   */

  zed.close();
  depth.close();
  depthRaw.close();
  rgb.close();
  return EXIT_SUCCESS;
}
