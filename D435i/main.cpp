//================================================================
// Created by João Vasco on 18/10/2021.
//================================================================

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

#include <fstream>              // File IO
#include <iostream>             // Terminal IO

const std::string rgbFile = "../D435i/rgbData.bin";
const std::string depthFile = "../D435i/depthData.bin";
const std::string depthRGBFile = "../D435i/depthRGBData.bin";


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
   *  Define the configuration for the camera, in this case we start the stream of:
   *  - RGB Color stream at 1280x720 @ 30fps
   *  - Depth Stream at 1280x720 @ 30fps
   *  and disable all others since we don't need them
   */

  rs2::config config;
  config.disable_all_streams();
  config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 1280, 720, rs2_format::RS2_FORMAT_RGB8, 30);
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 1280, 720, rs2_format::RS2_FORMAT_Z16, 30);

  /*
   * Create a color map to aply to the depth data,
   * in this case Black to White
   * i.e black is close white is far
   */

  rs2::colorizer color_map(3);

  /*
   * Create the data pipe to the camera with the configuration we want
   */
  rs2::pipeline pipe;
  pipe.start(config);

  /*
   * Open the binary files and check if they are really open
   */

  std::fstream rgb(rgbFile, std::ios::out | std::ios::binary);
  std::fstream depth(depthFile, std::ios::out | std::ios::binary);
  std::fstream depthRGB(depthRGBFile, std::ios::out | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open() || !depthRGB.is_open())
  {
    std::cout << "Failed while opening rgb or depth files." << std::endl;
    rgb.close();
    depth.close();
    return -1;
  }

  /*
   * This is, supposedly, to let the auto exposure settle a bit
   * Not sure if it really does something or not
   */

  for (auto i = 0; i < 30; ++i)
  {
    pipe.wait_for_frames();
  }

 /*
  * This is where we write the file start with info about the frame we will collect
  */

  for (auto &&dataFrame: pipe.wait_for_frames())
  {
    auto videoFrame = dataFrame.as<rs2::video_frame>();

    if (videoFrame.is<rs2::depth_frame>())
    {
      std::cout << "Depth" << std::endl;
      depthInfo.width = videoFrame.get_width();
      depthInfo.height = videoFrame.get_height();
      depthInfo.strideBytes = videoFrame.get_stride_in_bytes();
      depthInfo.pixelBytes = videoFrame.get_bytes_per_pixel();
      depthInfo.dataSize = videoFrame.get_data_size();

      videoFrame = color_map.process(dataFrame);

      std::cout << "Depth RGB" << std::endl;
      depthRGBInfo.width = videoFrame.get_width();
      depthRGBInfo.height = videoFrame.get_height();
      depthRGBInfo.strideBytes = videoFrame.get_stride_in_bytes();
      depthRGBInfo.pixelBytes = videoFrame.get_bytes_per_pixel();
      depthRGBInfo.dataSize = videoFrame.get_data_size();
    }
    else
    {
      std::cout << "Video" << std::endl;
      videoInfo.width = videoFrame.get_width();
      videoInfo.height = videoFrame.get_height();
      videoInfo.strideBytes = videoFrame.get_stride_in_bytes();
      videoInfo.pixelBytes = videoFrame.get_bytes_per_pixel();
      videoInfo.dataSize = videoFrame.get_data_size();
    }


  }
  /*
   * Print the data
   * Just for debug
   */
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
   * Save the structures to the files
   */

  depth.write(reinterpret_cast<char *>(&depthInfo), sizeof(depthInfo));
  depthRGB.write(reinterpret_cast<char *>(&depthRGBInfo), sizeof(depthRGBInfo));
  rgb.write(reinterpret_cast<char *>(&videoInfo), sizeof(videoInfo));


  int i = 0;

  /*
   * Capture 10 frames for test
   */

  for (int j = 0; j < 10; ++j)
  {
    for (auto &&frame: pipe.wait_for_frames())
    {
      std::cout << "Vai " << i << " Vezes" << std::endl;

      if (auto vf = frame.as<rs2::video_frame>())
      {
        if (vf.is<rs2::depth_frame>()) //If it is a depth frame
        {
          auto pixels = (uint16_t *) vf.get_data(); //Grab the data pointer
          depth.write(reinterpret_cast<char *>(pixels), vf.get_data_size()); //Write the raw data to a file

          vf = color_map.process(frame);  //Apply a color map and "Make an image"
          pixels = (uint16_t *) vf.get_data();   //Grab the data pointer
          depthRGB.write(reinterpret_cast<char *>(pixels), vf.get_data_size()); //Write the RGB Image to the file
        }
        else
        {
          auto pixels = (uint16_t *) vf.get_data(); //Grab the data pointer
          rgb.write(reinterpret_cast<char *>(pixels), vf.get_data_size()); //Save to a file
        }
      }
      i++;
    }
  }

  /*
   * Close all the files
   */

  rgb.close();
  depth.close();
  depthRGB.close();

  return 0;
}
