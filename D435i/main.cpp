// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams
#include <vector>

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

std::vector<rs2::video_frame> videoBuffer;
std::vector<rs2::depth_frame> depthBuffer;

int main(int argc, char *argv[]) try
{

  rs2::config config;
  config.disable_all_streams();
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 1280, 720, rs2_format::RS2_FORMAT_Z16, 15);
  config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 1280, 720, rs2_format::RS2_FORMAT_RGB8, 15);


  rs2::colorizer color_map;
  rs2::pipeline pipe;
  pipe.start(config);

  int counter = 0;
  int vfCounter = 0;
  int dfCounter = 0;


  for (int i = 0; i < 100; ++i)
  {

    auto &&frame = pipe.wait_for_frames();

    std::cout << "Frame: " << counter << std::endl;

    for (auto vframe: frame)
    {
      //frameBuffer.clear();
      if (rs2::video_frame vf = vframe.as<rs2::video_frame>())
      {

        if (vf.is<rs2::depth_frame>())
        {
          std::cout << "==========" << std::endl << "Depth" << std::endl << "==========" << std::endl;
          depthBuffer.emplace_back(vf);
          dfCounter++;
        }
        else
        {
          std::cout << "==========" << std::endl << "Video" << std::endl << "==========" << std::endl;
          videoBuffer.emplace_back(vf);
          vfCounter++;
        }
        std::cout << "Frame: " << counter << "  Video frame: " << vfCounter << "  Depth frame: " << dfCounter
                  << std::endl;

/*
      auto stream = frame.get_profile().stream_type();
      // Use the colorizer to get an rgb image for the depth stream
      if (vf.is<rs2::depth_frame>()) vf = color_map.process(frame);

      // Write images to disk
      std::stringstream png_file;
      png_file << "rs-save-to-disk-output-" << vf.get_profile().stream_name() << ".png";
      stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
                     vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
      std::cout << "Saved " << png_file.str() << std::endl;
*/
      }
    }
    counter++;
  }
  return EXIT_SUCCESS;
}
catch (const rs2::error &e)
{
  std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    "
            << e.what() << std::endl;
  return EXIT_FAILURE;
}
catch (const std::exception &e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}