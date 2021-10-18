//================================================================
// Created by João Vasco on 15/10/2021.
//================================================================
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <atomic>
#include <string>
#include <vector>
#include "include/utils.hpp" //Contains the ctrl+c handler

//Zed Libs
#include <sl/Camera.hpp>

//Intel Libs
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


std::atomic<int> zedFrameCount;
std::atomic<int> intelFrameCount;

std::vector<rs2::video_frame> intelFrameBuffer;

sl::String zedOutputPath("C:/Documentos 2/CLion/Cameras/Both/output/zed/test.svo");
std::string intelOutputPath = "C:\\Documentos 2\\CLion\\Cameras\\Both\\output\\intel\\";


/* Timing const so we now when to capture each frame
 * If we want a final output with 30 fps we need to
 * capture 30 frames per second ie one each 1000/30 milliseconds
 */
const int fpsOutput = 10;
const int millisBetweenFrames = 1000 / fpsOutput;

/* Aux functions declaration */
void threadTimer(std::function<void(sl::Camera &zedObject)> inputFunction,
                 std::function<void(rs2::pipeline &inputPipe, rs2::colorizer &inputColorizer)> inputFunction2,
                 unsigned int interval, sl::Camera &zedObject,
                 rs2::pipeline &inputPipe, rs2::colorizer &inputColorizer);
bool initZed(sl::Camera &zedObject);
void zedFrameCapture(sl::Camera &zedObject);
void intelFrameCapture(rs2::pipeline &inputPipe, rs2::colorizer &inputColorizer);


int main()
{
  std::cout << "[INFO] - Final FPS: " << fpsOutput << std::endl;
  std::cout << "[INFO] - Time between frames: " << millisBetweenFrames << std::endl;

  std::cout << "[INFO] - Starting Intel camera" << std::endl;

  rs2::config config;
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 1280, 720, rs2_format::RS2_FORMAT_Z16, 15);
  config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 1280, 720, rs2_format::RS2_FORMAT_RGB8, 15);

  rs2::colorizer intelColorMap(3); // Color Scheme white to black
  rs2::pipeline pipe;                       // Declare RealSense pipeline, encapsulating the actual device and sensors
  pipe.start(config);                             // Start streaming with provided configuration

  std::cout << "[INFO] - Intel camera ready" << std::endl;
  for (auto i = 0; i < 30; ++i)
  {
    pipe.wait_for_frames();
  }

  std::cout << "[INFO] - Starting Zed camera" << std::endl;
  sl::Camera zedCam;

  if (!initZed(zedCam))
  {
    return -1;
  }

  zedFrameCount = 0;

  //Put the cameras capturing in the background
  threadTimer(zedFrameCapture, intelFrameCapture, millisBetweenFrames, zedCam, pipe, intelColorMap);

  SetCtrlHandler(); //Capture CTRL + C so we know when to exit
  while (!exit_app); //Wait unitl we want to leave the app

  //======================
  //= App shutdown stuff =
  //======================

  //Wait for 2 seconds and let other threads to finish before closing
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  zedCam.disableRecording(); //Stop the video stream
  zedCam.close(); //Close the data stream and leave the cam available

  int counter = -1;

  for (auto frame: intelFrameBuffer)
  {
    if (auto vf = frame.as<rs2::video_frame>())
    {
      // Use the colorizer to get an rgb image for the depth stream
      if (vf.is<rs2::depth_frame>())
      {
        vf = intelColorMap.process(frame);
        counter++;
      }
      // Write images to disk
      std::stringstream png_file;
      png_file << intelOutputPath << "intel-" << counter << "-" << vf.get_profile().stream_name() << ".png";

      stbi_write_png(png_file.str().c_str(), vf.get_width(), vf.get_height(),
                     vf.get_bytes_per_pixel(), vf.get_data(), vf.get_stride_in_bytes());
    }
  }

  //Print some information and wait some time so we can read it
  std::cout << "===============================" << std::endl;
  std::cout << "[INFO ZED] - Frames Capturados: " << zedFrameCount << std::endl;
  std::cout << "[INFO INTEL] - Frames Capturados: " << intelFrameCount << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  return 0;
} //End main()

/*======================================================================================================================
 * initZed()
 * This function starts a zed cam object
 * with the parameters we want.
 * This function returns false if there was
 * any error and the camera couldnt be opened.
 * Returns true if everything is okay.
 *
 * Available parameters and values:
 * RESOLUTION sets the RGB image size
 * - HD2K   -> 2208*1242 15fps max
 * - HD1080 -> 1920*1080 30fps max
 * - HD720  -> 1280*720  60fps max
 * - VGA    -> 672*376  100fps max
 *
 * DEPTH_MODE sets the quality of the depth map
 * - ULTRA
 * - QUALITY
 * - PERFOMANCE
 * - NONE
 *====================================================================================================================*/
bool initZed(sl::Camera &zedObject)
{
  sl::InitParameters init_parameters;
  //Cam initial parameters
  init_parameters.camera_resolution = sl::RESOLUTION::HD720;
  init_parameters.camera_fps = 60;
  init_parameters.depth_mode = sl::DEPTH_MODE::QUALITY;

  auto returned_state = zedObject.open(init_parameters); //Open the camera data stream

  if (returned_state != sl::ERROR_CODE::SUCCESS) // If it failed print the error message and exit
  {
    std::cout << "[FATAL ERROR] - Error starting Zed. " << sl::toVerbose(returned_state) << std::endl;
    return false;
  }
  else
  {
    std::cout << "[INFO] - Zed camera started with success" << std::endl;
  }

  // Start the video recording
  // If you set the compression mode to H264 or H265 you will need a compatible NVIDIA GPU

  sl::RecordingParameters rec_parameters;
  rec_parameters.video_filename = zedOutputPath;
  rec_parameters.compression_mode = sl::SVO_COMPRESSION_MODE::LOSSLESS;

  returned_state = zedObject.enableRecording(rec_parameters);

  if (returned_state != sl::ERROR_CODE::SUCCESS)// If it failed print the error message and exit
  {
    std::cout << "[FATAL ERROR] - Error starting recording. " << sl::toVerbose(returned_state) << std::endl;
    zedObject.close();
    return false;
  }
  else
  {
    std::cout << "[INFO] - Zed camera ready to record" << std::endl;
  }

  return true;
}

/*======================================================================================================================
 *====================================================================================================================*/
void threadTimer(std::function<void(sl::Camera &zedObject)> inputFunction,
                 std::function<void(rs2::pipeline &inputPipe, rs2::colorizer &inputColorizer)> inputFunction2,
                 unsigned int interval, sl::Camera &zedObject, rs2::pipeline &inputPipe, rs2::colorizer &inputColorizer)
{
  std::thread([inputFunction, inputFunction2, interval, &zedObject, &inputPipe, &inputColorizer]()
              {
                  while (true)
                  {
                    auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
                    inputFunction(zedObject);
                    inputFunction2(inputPipe, inputColorizer);
                    std::this_thread::sleep_until(x);
                  }
              }).
    detach();
}

/*======================================================================================================================
 *====================================================================================================================*/
void zedFrameCapture(sl::Camera &zedObject)
{
  //sl::RecordingStatus rec_status;
  if (zedObject.grab() == sl::ERROR_CODE::SUCCESS)
  {
    zedFrameCount++;
    std::cout << "[INFO ZED] - Frame count: " << zedFrameCount << std::endl;
  }
}

/*======================================================================================================================
 *====================================================================================================================*/
void intelFrameCapture(rs2::pipeline &inputPipe, rs2::colorizer &inputColorizer) try
{
  for (auto &&frame: inputPipe.wait_for_frames(1000))
  {
    intelFrameBuffer.emplace_back(frame);
  }
  intelFrameCount++;
  std::cout << "[INFO INTEL] - Frame count: " << intelFrameCount << std::endl;
}
catch (const rs2::error &e)
{
  std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    "
            << e.what() << std::endl;
}
catch (const std::exception &e)
{
  std::cerr << e.what() << std::endl;
}
/*======================================================================================================================
 *====================================================================================================================*/
