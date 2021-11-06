//================================================================
// Created by João Vasco on 05/10/2021.
//================================================================

// ZED includes
#include <sl/Camera.hpp>
#include "include/utils.hpp"
#include <opencv2/opencv.hpp>


using namespace sl;
using namespace std;

void print(string msg_prefix, ERROR_CODE err_code = ERROR_CODE::SUCCESS, string msg_suffix = "");

enum APP_TYPE
{
    LEFT_AND_RIGHT,
    LEFT_AND_DEPTH,
    LEFT_AND_DEPTH_16
};

//String outputPath("C:/Documentos 2/CLion/Cameras/ZED/output/");
string outputPath;

int main(int argc, char **argv)
{
  cout << "Recolha de imagens ZED 2" << endl;
  cout << "Caminho de saida absoluto ou ./ para este diretorio" << endl;
  cout << "Obrigatorio um / no fim. Default ./output/" << endl;

  if (cin.peek() == '\n')
  {
    outputPath = "./output/";
  }
  else if (!(std::cin >> outputPath))
  { //be sure to handle invalid input
    cout << "Invalid input.\n";
    //error handling
  }

  // Create a ZED camera
  Camera zed;

  /* Set configuration parameters for the ZED
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
   *
   */


  InitParameters init_parameters;
  init_parameters.camera_resolution = RESOLUTION::HD720;
  init_parameters.camera_fps = 15;
  init_parameters.depth_mode = DEPTH_MODE::QUALITY;

  RecordingParameters recording_parameters;

  recording_parameters.video_filename.set(outputPath.c_str());
  recording_parameters.compression_mode = SVO_COMPRESSION_MODE::LOSSLESS;

  // Open the camera
  auto returned_state = zed.open(init_parameters);
  if (returned_state != ERROR_CODE::SUCCESS)
  {
    print("Camera Open", returned_state, "Exit program.");
    return EXIT_FAILURE;
  }

  // Enable recording with the filename specified in argument
  String path_output(argv[1]);
  returned_state = zed.enableRecording(recording_parameters);

  if (returned_state != ERROR_CODE::SUCCESS)
  {
    print("Recording ZED : ", returned_state);
    zed.close();
    return EXIT_FAILURE;
  }

  // Start recording SVO, stop with Ctrl-C command
  print("SVO is Recording, use Ctrl-C to stop.");
  SetCtrlHandler();
  int frames_recorded = 0;
  sl::RecordingStatus rec_status;
  while (!exit_app)
  {
    if (zed.grab() == ERROR_CODE::SUCCESS)
    {
      // Each new frame is added to the SVO file
      rec_status = zed.getRecordingStatus();
      if (rec_status.status)
        frames_recorded++;
      std::cout << '\r' << "Frame Count: " << frames_recorded << "Ctrl + C para sair" << std::flush;
    }
  }

  // Stop recording
  zed.disableRecording();
  zed.close();

  //################################
  //# Parsing the SVO file to pngs #
  //################################

  // Get input parameters
  string svo_input_path(argv[1]);
  string output_path(argv[2]);
  APP_TYPE app_type = LEFT_AND_RIGHT;
  if (!strcmp(argv[3], "1") || !strcmp(argv[3], "3"))
    app_type = LEFT_AND_DEPTH;
  if (!strcmp(argv[3], "4"))
    app_type = LEFT_AND_DEPTH_16;

  // Check if exporting to AVI or SEQUENCEs

  if (!directoryExists(output_path))
  {
    print("Input directory doesn't exist. Check permissions or create it." + output_path);
    return EXIT_FAILURE;
  }

  if (output_path.back() != '/' && output_path.back() != '\\')
  {
    print("Error: output folder needs to end with '/' or '\\'." + output_path);
    return EXIT_FAILURE;
  }

  // Create ZED objects
  Camera zed;

  // Specify SVO path parameter
  InitParameters init_parameters;
  init_parameters.input.setFromSVOFile(svo_input_path.c_str());
  init_parameters.coordinate_units = UNIT::MILLIMETER;

  // Open the camera
  ERROR_CODE zed_open_state = zed.open(init_parameters);
  if (zed_open_state != ERROR_CODE::SUCCESS)
  {
    print("Camera Open", zed_open_state, "Exit program.");
    return EXIT_FAILURE;
  }

  // Get image size
  Resolution image_size = zed.getCameraInformation().camera_configuration.resolution;

  Mat left_image(image_size, MAT_TYPE::U8_C4);
  cv::Mat left_image_ocv = slMat2cvMat(left_image);

  Mat right_image(image_size, MAT_TYPE::U8_C4);
  cv::Mat right_image_ocv = slMat2cvMat(right_image);

  Mat depth_image(image_size, MAT_TYPE::F32_C1);
  cv::Mat depth_image_ocv = slMat2cvMat(depth_image);

  cv::Mat image_sideByside;

  // Create video writer
  cv::VideoWriter video_writer;

  RuntimeParameters rt_param;
  rt_param.sensing_mode = SENSING_MODE::FILL;

  // Start SVO conversion to AVI/SEQUENCE
  print("Converting SVO... Use Ctrl-C to interrupt conversion.");

  int nb_frames = zed.getSVONumberOfFrames();
  int svo_position = 0;
  zed.setSVOPosition(svo_position);

  SetCtrlHandler();

  while (!exit_app)
  {
    sl::ERROR_CODE err = zed.grab(rt_param);
    if (err == ERROR_CODE::SUCCESS)
    {
      svo_position = zed.getSVOPosition();

      // Retrieve SVO images
      zed.retrieveImage(left_image, VIEW::LEFT);

      switch (app_type)
      {
        case LEFT_AND_RIGHT:
          zed.retrieveImage(right_image, VIEW::RIGHT);
          break;
        case LEFT_AND_DEPTH:
          zed.retrieveImage(right_image, VIEW::DEPTH);
          break;
        case LEFT_AND_DEPTH_16:
          zed.retrieveMeasure(depth_image, MEASURE::DEPTH);
          break;
        default:
          break;
      }


      // Generate filenames
      ostringstream filename1;
      filename1 << output_path << "/left" << setfill('0') << setw(6) << svo_position << ".png";
      ostringstream filename2;
      filename2 << output_path << (app_type == LEFT_AND_RIGHT ? "/right" : "/depth") << setfill('0') << setw(6)
                << svo_position << ".png";

      // Save Left images
      cv::imwrite(filename1.str(), left_image_ocv);

      // Save depth
      if (app_type != LEFT_AND_DEPTH_16)
        cv::imwrite(filename2.str(), right_image_ocv);
      else
      {
        // Convert to 16Bit
        cv::Mat depth16;
        depth_image_ocv.convertTo(depth16, CV_16UC1);
        cv::imwrite(filename2.str(), depth16);
      }


      // Display progress
      ProgressBar((float) (svo_position / (float) nb_frames), 30);
    }
    else if (err == sl::ERROR_CODE::END_OF_SVOFILE_REACHED)
    {
      print("SVO end has been reached. Exiting now.");
      exit_app = true;
    }
    else
    {
      print("Grab Error: ", err);
      exit_app = true;
    }
  }

  zed.close();
  return EXIT_SUCCESS;

  return EXIT_SUCCESS;
}

void print(string msg_prefix, ERROR_CODE err_code, string msg_suffix)
{
  cout << "[Sample]";
  if (err_code != ERROR_CODE::SUCCESS)
    cout << "[Error] ";
  else
    cout << " ";
  cout << msg_prefix << " ";
  if (err_code != ERROR_CODE::SUCCESS)
  {
    cout << " | " << toString(err_code) << " : ";
    cout << toVerbose(err_code);
  }
  if (!msg_suffix.empty())
    cout << " " << msg_suffix;
  cout << endl;
}
