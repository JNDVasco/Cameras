//================================================================
// Created by João Vasco on 23/09/2021.
//================================================================
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <string>
#include <chrono>               // Time

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <experimental/filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

namespace fs = std::experimental::filesystem;
using namespace std;

long millis();

string outFolder = "Output\\";
// string outRGB = "RGB\\";
// string outDepth = "Depth\\";
string filepath = "C:\\Documentos 2\\CLion\\Cameras\\";

const int photoCount = 20; //Number of frames to capture

int main(int argc, char *argv[]) try
{
  long captureTimeLog[photoCount];

  outFolder = filepath + outFolder;

  if (!fs::is_directory(outFolder) || !fs::exists(outFolder))
  { // Check if src folder exists
    fs::create_directory(outFolder); // create src folder
    cout << "Pasta Criada em: " << outFolder << endl;
  }

  rs2::colorizer color_map(3);  // Declare depth colorizer for pretty visualization of depth data
  rs2::pipeline pipe; // Declare RealSense pipeline, encapsulating the actual device and sensors
  pipe.start();     // Start streaming with default recommended configuration

  for (auto i = 0; i < 30; ++i) pipe.wait_for_frames(); //Let the cam output stabilize

  long startMillis = millis();
  long oldMillis = startMillis;

  for (int i = 0; i < 20; ++i)
  {
    oldMillis = millis();
    for (auto &&frame: pipe.wait_for_frames())
    {
      // We can only save video frames as pngs, so we skip the rest
      if (auto vf = frame.as<rs2::video_frame>())
      {
        if (vf.is<rs2::depth_frame>()) continue;
        //if (vf.is<rs2::depth_frame>()) vf = color_map.process(frame);

        stringstream png_file;
        png_file << outFolder << "RS_D435i_" << (millis() - startMillis) << vf.get_profile().stream_name() << ".png";
        stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
                       vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
        cout << "Saved " << png_file.str() << endl;
      }
      captureTimeLog[i] = millis() - oldMillis;
    }
  }
  cout << millis() << endl;

  cout << "==== Duracoes ====" << endl;

  for (int i = 0; i < photoCount; ++i)
  {
    cout << "Duracao " << i << " :" << captureTimeLog[i] << endl;
  }

  return EXIT_SUCCESS;
}
catch (const rs2::error &e)
{
  cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    "
       << e.what() << endl;
  return EXIT_FAILURE;
}
catch (const exception &e)
{
  cerr << e.what() << endl;
  return EXIT_FAILURE;
}


//======================================================================
//= This calculates the current millis
//= Receives nothing
//= Returns the current milliseconds
//= Status: Done
//======================================================================
long millis()
{
  return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}//End millis