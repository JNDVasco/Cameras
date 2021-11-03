//================================================================
// Created by João Vasco on 15/10/2021.
//================================================================
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <atomic>
#include <string>
#include "include/utils.hpp" //Contains the ctrl+c handler
#include <fstream>

//Zed Libs
#include <sl/Camera.hpp>

//Intel Libs
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

std::atomic<int> zedFrameCount;
std::atomic<int> intelFrameCount;

sl::String zedOutputPath("C:/Documentos 2/CLion/Cameras/Both/output/zed/test.svo");
std::string intelOutputPath = "C:\\Documentos 2\\CLion\\Cameras\\Both\\output\\intel\\";

const std::string rgbFile = "../Both/output/intel/rgbData.bin";
const std::string depthFile = "../Both/output/intel/depthData.bin";
const std::string depthRGBFile = "../Both/output/intel/depthRGBData.bin";


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


/* Timing const so we now when to capture each frame
 * If we want a final output with 30 fps we need to
 * capture 30 frames per second ie one each 1000/30 milliseconds
 */
const int fpsOutput = 30;
const int millisBetweenFrames = 1000 / fpsOutput;


/* Aux functions declaration */
void threadTimer(std::function<void(sl::Camera &)> inputFunctionZed,
                 std::function<void(rs2::pipeline, rs2::colorizer, std::fstream &, std::fstream &,
                                    std::fstream &)> inputFunctionIntel,
                 unsigned int interval, sl::Camera &zedObject, rs2::pipeline &inputPipe,
                 rs2::colorizer &inputColorizer, std::fstream &rgbFile, std::fstream &depthFile,
                 std::fstream &depthRGBFile);
void intelFrameCapture(rs2::pipeline inputPipe, rs2::colorizer inputColorizer, std::fstream &rgbFile,
                       std::fstream &depthFile, std::fstream &depthRGBFile);
bool initZed(sl::Camera &zedObject);
void zedFrameCapture(sl::Camera &zedObject);

int main()
{
  frameInfo depthInfo;
  frameInfo depthRGBInfo;
  frameInfo videoInfo;

  std::cout << "[INFO] - Final FPS: " << fpsOutput << std::endl;
  std::cout << "[INFO] - Time between frames: " << millisBetweenFrames << std::endl;

  std::cout << "[INFO] - Starting Zed camera" << std::endl;

  sl::Camera zedCam;

  if (!initZed(zedCam))
  {
    return -1;
  }

  std::cout << "[INFO] - Starting Intel camera" << std::endl;

  rs2::config config;
  config.disable_all_streams();
  config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 1280, 720, rs2_format::RS2_FORMAT_RGB8, 30);
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 1280, 720, rs2_format::RS2_FORMAT_Z16, 30);

  rs2::colorizer intelColorMap(3);
  rs2::pipeline pipe;
  pipe.start(config);

  std::fstream rgb(rgbFile, std::ios::out | std::ios::binary);
  std::fstream depth(depthFile, std::ios::out | std::ios::binary);
  std::fstream depthRGB(depthRGBFile, std::ios::out | std::ios::binary);

  if (!rgb.is_open() || !depth.is_open() || !depthRGB.is_open())
  {
    std::cout << "Failed while opening rgb or depth files." << std::endl;
    rgb.close();
    depth.close();
    depthRGB.close();
    return -1;
  }
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

      videoFrame = intelColorMap.process(dataFrame);

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

  std::cout << std::endl << "Depth size: " << sizeof(depthInfo) << "  Depth RGB size: "
            << sizeof(depthRGBInfo) << "  Video size: " << sizeof(videoInfo) << std::endl;
  std::cout << "====================" << std::endl << std::endl;

  depth.write(reinterpret_cast<char *>(&depthInfo), sizeof(depthInfo));
  depthRGB.write(reinterpret_cast<char *>(&depthRGBInfo), sizeof(depthRGBInfo));
  rgb.write(reinterpret_cast<char *>(&videoInfo), sizeof(videoInfo));

  std::cout << "[INFO] - Intel camera ready" << std::endl;


  //Put the cameras capturing in the background
  threadTimer(zedFrameCapture, intelFrameCapture, millisBetweenFrames, zedCam, pipe, intelColorMap, rgb, depth , depthRGB);

  SetCtrlHandler(); //Capture CTRL + C so we know when to exit
  while (!exit_app); //Wait unitl we want to leave the app

  //======================
  //= App shutdown stuff =
  //======================

  //Wait for 2 seconds and let other threads to finish before closing
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  zedCam.disableRecording(); //Stop the video stream
  zedCam.close(); //Close the data stream and leave the cam available

  rgb.close();
  depth.close();
  depthRGB.close();

  //Print some information and wait some time so we can read it
  std::cout << "===================================" << std::endl;
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
void threadTimer(std::function<void(sl::Camera &)> inputFunctionZed,
                 std::function<void(rs2::pipeline, rs2::colorizer)> inputFunctionIntel,
                 unsigned int interval, sl::Camera &zedObject, rs2::pipeline &inputPipe,
                 rs2::colorizer &inputColorizer)
{
  std::thread([inputFunctionZed, inputFunctionIntel, interval, &zedObject, &inputPipe, &inputColorizer]()
              {
                  while (!exit_app)
                  {
                    auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);

                    inputFunctionZed(zedObject);

                    std::this_thread::sleep_until(x);
                  }
              }).detach();
}

/*======================================================================================================================
 *====================================================================================================================*/
void zedFrameCapture(sl::Camera &zedObject)
{
  //sl::RecordingStatus rec_status;
  if (zedObject.grab() == sl::ERROR_CODE::SUCCESS)
  {
    zedFrameCount++;
    if (zedFrameCount % 10)
    {
      std::cout << "[INFO ZED] - Frame count: " << zedFrameCount << std::endl;
    }
  }
}

/*======================================================================================================================
 *====================================================================================================================*/
void intelFrameCapture(rs2::pipeline inputPipe, rs2::colorizer inputColorizer)
{
  for (auto &&frame: inputPipe.wait_for_frames())
  {
    if (auto vf = frame.as<rs2::video_frame>())
    {
      if (vf.is<rs2::depth_frame>())
      {
        auto pixels = (uint16_t *) vf.get_data();
        depth.write(reinterpret_cast<char *>(pixels), vf.get_data_size());

        vf = inputColorizer.process(frame);
        pixels = (uint16_t *) vf.get_data();
        depthRGB.write(reinterpret_cast<char *>(pixels), vf.get_data_size());
      }
      else
      {
        auto pixels = (uint16_t *) vf.get_data();
        rgb.write(reinterpret_cast<char *>(pixels), vf.get_data_size());
      }
    }
  }
  intelFrameCount++;
  if (intelFrameCount % 10)
  {
    std::cout << "[INFO INTEL] - Frame count: " << intelFrameCount << std::endl;
  }
}