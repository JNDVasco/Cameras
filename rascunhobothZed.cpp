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
const int fpsOutput = 20;
const int millisBetweenFrames = 1000 / fpsOutput;

/* Aux functions declaration */
void
threadTimer(std::function<void(sl::Camera &zedObject)> inputFunction, unsigned int interval, sl::Camera &zedObject);
bool initZed(sl::Camera &zedObject);
void zedFrameCapture(sl::Camera &zedObject);


int main()
{
  std::cout << "[INFO] - Final FPS: " << fpsOutput << std::endl;
  std::cout << "[INFO] - Time between frames: " << millisBetweenFrames << std::endl;


  std::cout << "[INFO] - Starting Zed camera" << std::endl;
  sl::Camera zedCam;

  if (!initZed(zedCam))
  {
    return -1;
  }

  zedFrameCount = 0;

  //Put the cameras capturing in the background
  threadTimer(zedFrameCapture, millisBetweenFrames, zedCam);

  SetCtrlHandler(); //Capture CTRL + C so we know when to exit
  while (!exit_app); //Wait unitl we want to leave the app

  //======================
  //= App shutdown stuff =
  //======================

  //Wait for 2 seconds and let other threads to finish before closing
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  zedCam.disableRecording(); //Stop the video stream
  zedCam.close(); //Close the data stream and leave the cam available

  //Print some information and wait some time so we can read it
  std::cout << "===================================" << std::endl;
  std::cout << "[INFO ZED] - Frames Capturados: " << zedFrameCount << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));

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
void threadTimer(std::function<void(sl::Camera &zedObject)> inputFunction, unsigned int interval, sl::Camera &zedObject)
{
  std::thread([inputFunction, interval, &zedObject]()
              {
                  while (!exit_app)
                  {
                    auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
                    inputFunction(zedObject);
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
