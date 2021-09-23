//================================================================
// Created by João Vasco on 23/09/2021.
//================================================================
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <fstream>              // File IO
#include <iostream>             // Terminal IO
#include <sstream>              // Stringstreams
#include <chrono>               // Time

using namespace std;

// 3rd party header for writing png files
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

long millis();

// This sample captures 30 frames and writes the last frame to disk.
// It can be useful for debugging an embedded system with no display.
int main(int argc, char *argv[]) try
{
    cout << "Passei aqui" << endl;

    cout << millis() << endl;

    rs2::colorizer color_map;  // Declare depth colorizer for pretty visualization of depth data
    rs2::pipeline pipe; // Declare RealSense pipeline, encapsulating the actual device and sensors
    pipe.start();     // Start streaming with default recommended configuration

    // Capture 30 frames to give autoexposure, etc. a chance to settle
    for (auto i = 0; i < 30; ++i) pipe.wait_for_frames();

    // Wait for the next set of frames from the camera. Now that autoexposure, etc.
    // has settled, we will write these to disk
    for (auto &&frame: pipe.wait_for_frames())
    {
        // We can only save video frames as pngs, so we skip the rest
        if (auto vf = frame.as<rs2::video_frame>())
        {
            // Use the colorizer to get an rgb image for the depth stream
            if (vf.is<rs2::depth_frame>()) vf = color_map.process(frame);

            // Write images to disk
            stringstream png_file;
            png_file << "rs-save-to-disk-output-" << vf.get_profile().stream_name() << ".png";
            stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
                           vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
            cout << "Saved " << png_file.str() << endl;
        }
    }
    cout<< millis() << endl;

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