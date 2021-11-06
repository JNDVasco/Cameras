//================================================================
// Created by João Vasco on 15/10/2021.
//================================================================

#include <sl/Camera.hpp>
#include <fstream>              // File IO
#include <iostream>             // Terminal IO

const std::string rgbFile = "../ZED/data/rgbData.bin";
const std::string depthFile = "../ZED/data/depthData.bin";

int main(int argc, char **argv)
{
  /*
   * Open the files and check if they are open
   */

  std::fstream rgb(rgbFile, std::ios::out | std::ios::binary);
  std::fstream depth(depthFile, std::ios::out | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open())
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


  sl::Mat rgbImage(imageSize, sl::MAT_TYPE::U8_C4);

  std::cout << "W: " << imageSize.width << "  H: " << imageSize.height << "  Bytes per Row: " << rgbImage.getWidthBytes()
       << " Bytes per Pixel: " << rgbImage.getPixelBytes() << " Total Size: "
       << rgbImage.getWidthBytes() * imageSize.height << std::endl;

  sl::Mat depthImage(imageSize, sl::MAT_TYPE::F32_C1);

  std::cout << "W: " << imageSize.width << "  H: " << imageSize.height << "  Bytes per Row: " << depthImage.getWidthBytes()
       << " Bytes per Pixel: " << depthImage.getPixelBytes() << " Total Size: "
       << depthImage.getWidthBytes() * imageSize.height
       << std::endl;

  for (int i = 0; i < 50; ++i)
  {
    // Grab an image
    returned_state = zed.grab();
    // A new image is available if grab() returns ERROR_CODE::SUCCESS
    if (returned_state == sl::ERROR_CODE::SUCCESS)
    {

      // Get the left image
      zed.retrieveImage(rgbImage, sl::VIEW::LEFT);
      std::cout << "Frame!" << std::endl;
      auto pixels = rgbImage.getPtr<sl::uchar1>();
      depth.write(reinterpret_cast<char *>(pixels), (rgbImage.getWidthBytes() * imageSize.width)); //Save to a file

      // Display the image resolution and its acquisition timestampcout << "Image resolution: " << rgbImage.getWidth() << "x" << rgbImage.getHeight() << " || Image timestamp: "
      //<< rgbImage.timestamp.data_ns << endl;
    }
  }

  /*
   * Close the camera and the files
   */

  zed.close();
  depth.close();
  rgb.close();
  return EXIT_SUCCESS;
}
