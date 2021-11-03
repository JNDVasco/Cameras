// ZED includes
#include <sl/Camera.hpp>

// Sample includes
#include "include/utils.hpp"

// Using namespace
using namespace sl;
using namespace std;

int main(int argc, char **argv)
{

  if (argc < 2)
  {
    cout << "Usage : Only the path of the output SVO file should be passed as argument.\n";
    return EXIT_FAILURE;
  }

  // Create a ZED camera
  Camera zed;

  // Set configuration parameters for the ZED
  InitParameters init_parameters;
  init_parameters.camera_resolution = RESOLUTION::HD1080;
  init_parameters.camera_fps = 30;
  init_parameters.depth_mode = DEPTH_MODE::QUALITY;

  // Open the camera
  auto returned_state = zed.open(init_parameters);
  if (returned_state != ERROR_CODE::SUCCESS) {
    cout << "Error " << returned_state << ", exit program." << endl;
    return EXIT_FAILURE;
  }

  // Capture 50 frames and stop
  int i = 0;

  sl::Mat image;
  while (i < 50) {
    // Grab an image
    returned_state = zed.grab();
    // A new image is available if grab() returns ERROR_CODE::SUCCESS
    if (returned_state == ERROR_CODE::SUCCESS) {

      // Get the left image
      zed.retrieveImage(image, VIEW::LEFT);

      // Display the image resolution and its acquisition timestamp
      cout<<"Image resolution: "<< image.getWidth()<<"x"<<image.getHeight() <<" || Image timestamp: "<<image.timestamp.data_ns<<endl;
      i++;
    }
  }

  // Close the camera
  zed.close();
  return EXIT_SUCCESS;
}
