// ZED includes
#include <sl/Camera.hpp>
#include <fstream>              // File IO
#include <iostream>             // Terminal IO

const std::string rgbFile = "../ZED/data/rgbData.bin";
const std::string depthFile = "../ZED/data/depthData.bin";

// Using namespace
using namespace sl;
using namespace std;

int main(int argc, char **argv)
{
  std::fstream rgb(rgbFile, std::ios::out | std::ios::binary);
  std::fstream depth(depthFile, std::ios::out | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open())
  {
    std::cout << "Failed while opening rgb or depth files." << std::endl;
    rgb.close();
    depth.close();
    return -1;
  }

  Camera zed;

  // Set configuration parameters for the ZED
  InitParameters init_parameters;
  init_parameters.camera_resolution = RESOLUTION::HD720;
  init_parameters.camera_fps = 30;
  init_parameters.depth_mode = DEPTH_MODE::QUALITY;

  // Open the camera
  auto returned_state = zed.open(init_parameters);
  if (returned_state != ERROR_CODE::SUCCESS)
  {
    cout << "Error " << returned_state << ", exit program." << endl;
    return EXIT_FAILURE;
  }


  Resolution imageSize = zed.getCameraInformation().camera_configuration.resolution;

  // Capture 50 frames and stop
  int i = 0;

  sl::Mat rgbImage(imageSize, sl::MAT_TYPE::U8_C4);

  cout << "W: " << imageSize.width << "  H: " << imageSize.height << "  Bytes per Row: " << rgbImage.getWidthBytes()
       << " Bytes per Pixel: " << rgbImage.getPixelBytes() << " Total Size: " << rgbImage.getWidthBytes() * imageSize.width
       << endl;

  Mat depthImage(imageSize, MAT_TYPE::F32_C1);

  cout << "W: " << imageSize.width << "  H: " << imageSize.height << "  Bytes per Row: " << depthImage.getWidthBytes()
       << " Bytes per Pixel: " << depthImage.getPixelBytes() << " Total Size: " << depthImage.getWidthBytes() * imageSize.height
       << endl;

  while (i < 50)
  {
    // Grab an image
    returned_state = zed.grab();
    // A new image is available if grab() returns ERROR_CODE::SUCCESS
    if (returned_state == ERROR_CODE::SUCCESS)
    {

      // Get the left image
      zed.retrieveImage(rgbImage, VIEW::LEFT);

      // Display the image resolution and its acquisition timestampcout << "Image resolution: " << rgbImage.getWidth() << "x" << rgbImage.getHeight() << " || Image timestamp: "
           << rgbImage.timestamp.data_ns << endl;
      i++;
    }
  }

  // Close the camera
  zed.close();
  return EXIT_SUCCESS;
}
