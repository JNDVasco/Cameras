//================================================================
// Created by João Vasco on 15/10/2021.
//================================================================
#include <iostream>
#include <thread>
#include <chrono>

//Zed Libs
#include <sl/Camera.hpp>


/* Timing const so we now when to capture each frame
 * If we want a final output with 30 fps we need to
 * capture 30 frames per second ie one each 1000/30 milliseconds
 */
const int fpsOutput = 30;
const int millisBetweenFrames = 1000 / fpsOutput;

/* Aux functions declaration */
bool initZed(sl::Camera &zedObject);
void initIntel();
void threadOne();
void threadTwo();


int main()
{
  std::cout << "Final FPS: " << fpsOutput << std::endl;

  std::cout << "Starting Zed camera" << std::endl;
  sl::Camera zedCam;

  if (!initZed(zedCam))
  {
    return -1;
  }

  std::cout << "Starting Intel camera" << std::endl;
  sl::Camera intelCam;

  if (!initZed(zedCam))
  {
    return -1;
  }


  std::thread th1(threadOne);
  std::thread th2(threadTwo);


  th1.join();
  th2.join();
  return 0;
} //End main()

/* initZed()
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
 */
bool initZed(sl::Camera &zedObject)
{
  sl::InitParameters init_parameters;
  //Cam initial parameters
  init_parameters.camera_resolution = sl::RESOLUTION::HD720;
  init_parameters.camera_fps = 60;
  init_parameters.depth_mode = sl::DEPTH_MODE::QUALITY;

  auto returned_state = zedObject.open(init_parameters); //Open the camera data stream

  if (returned_state != sl::ERROR_CODE::SUCCESS)
  {
    std::cout << "[FATAL ERROR] - Error starting Zed. " << sl::toVerbose(returned_state) << std::endl;
    return false;
  }
  else
  {
    std::cout << "[INFO] - Zed camera started with sucess" << std::endl;
    return true;
  }
}


void threadOne()
{
  std::cout << "Printed from thread one!" << std::endl;
}

void threadTwo()
{
  std::cout << "Printed from thread two!" << std::endl;
}