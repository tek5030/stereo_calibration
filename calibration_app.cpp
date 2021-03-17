#include "stereo_calib.h"
#include "tek5030/realsense_stereo.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace tek5030;
using namespace tek5030::RealSense;

// Define a few font parameters for convenience.
namespace font
{
constexpr auto face = cv::FONT_HERSHEY_PLAIN;
constexpr auto scale = 1.0;
}

// Define a few BGR-colors for convenience.
namespace color
{
const cv::Scalar green(0, 255, 0);
const cv::Scalar red(0, 0, 255);
}

void addHelpText(cv::Mat& img);

void addNumberOfImagesText(cv::Mat& img, size_t num_images);

std::vector<StereoPair>
captureStereoImages(const StereoCamera& stereo_camera, const cv::Size& chessboard_size);

cv::Mat findAndDrawChessboard(const cv::Size& chessboard_size, const cv::Mat& frame);

void addChessboardVisualization(cv::Mat& view, const cv::Size& chessboard_size, const StereoPair& stereo_pair);

std::vector<std::string> writeImages(const std::vector<StereoPair>& stereo_images);

int main() try
{
  StereoCamera stereo_camera(StereoCamera::CaptureMode::UNRECTIFIED);
  stereo_camera.setLaserMode(StereoCamera::LaserMode::OFF);

  const cv::Size chessboard_size = {8, 5};
  constexpr float square_size = 0.03f;

  const auto stereo_images = captureStereoImages(stereo_camera, chessboard_size);

  // Write the images to files, and retreive filenames.
  const auto img_filenames = writeImages(stereo_images);

  // Run the stereo calibration using the filenames.
  StereoCalib(img_filenames, chessboard_size, square_size, false, true, true);
}
catch (const std::exception& e)
{
  std::cerr << "Caught exception:\n"
            << e.what() << "\n";
  return EXIT_FAILURE;
}
catch (...)
{
  std::cerr << "Caught unknown exception\n";
  return EXIT_FAILURE;
}

void addHelpText(cv::Mat& img)
{
  cv::putText(
    img,
    {"Press [space] to capture pair, [q] to save and quit"},
    {10, 20},
    font::face,
    font::scale,
    color::red
  );
}

void addNumberOfImagesText(cv::Mat& img, size_t num_images)
{
  cv::putText(
    img,
    {"Number of captured images: " + std::to_string(num_images)},
    {10, 40},
    font::face,
    font::scale,
    color::red
  );
}

cv::Mat findAndDrawChessboard(const cv::Size& chessboard_size, const cv::Mat& frame)
{
  constexpr int chessBoardFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK;

  cv::Mat corners;
  const bool found = findChessboardCorners(frame, chessboard_size, corners, chessBoardFlags);

  cv::Mat view;
  cv::cvtColor(frame, view, cv::COLOR_GRAY2BGR);
  cv::drawChessboardCorners(view, chessboard_size, corners, found);

  return view;
}

void addChessboardVisualization(cv::Mat& view, const cv::Size& chessboard_size, const StereoPair& stereo_pair)
{
  cv::hconcat(
    findAndDrawChessboard(chessboard_size, stereo_pair.left),
    findAndDrawChessboard(chessboard_size, stereo_pair.right),
    view);

  cv::putText(view, "LEFT", {10,60}, font::face, font::scale, color::green);
  cv::putText(view, "RIGHT", { view.cols / 2 + 10, 60 }, font::face, font::scale, color::green);
}

std::vector<std::string> writeImages(const std::vector<StereoPair>& stereo_images)
{
  std::vector<std::string> img_filenames;

  for (size_t i = 0; i < stereo_images.size(); ++i)
  {
    std::stringstream left_filename;
    left_filename << "stereo_left_" << i << ".png";
    cv::imwrite(left_filename.str(), stereo_images[i].left);
    img_filenames.push_back(left_filename.str());

    std::stringstream right_filename;
    right_filename << "stereo_right_" << i << ".png";
    cv::imwrite(right_filename.str(), stereo_images[i].right);
    img_filenames.push_back(right_filename.str());
  }

  return img_filenames;
}

std::vector<StereoPair> captureStereoImages(
  const StereoCamera& stereo_camera,
  const cv::Size& chessboard_size
)
{
  const std::string win_name{"Stereo capture"};
  cv::namedWindow(win_name, cv::WINDOW_NORMAL);

  std::vector<StereoPair> stereo_images;
  constexpr size_t maximum_number_of_pairs = 100;
  for (; stereo_images.size() < maximum_number_of_pairs;)
  {
    StereoPair stereo_img = stereo_camera.getStereoPair();

    // Convert to UINT8 images.
    stereo_img.left.convertTo(stereo_img.left, CV_8UC1, 255.0/65535.0);
    stereo_img.right.convertTo(stereo_img.right, CV_8UC1, 255.0/65535.0);

    // Find chessboard. The corners will not be used, but will at least give an impression of the applicability
    // of the stereo capture for calibration.
    cv::Mat view;
    addChessboardVisualization(view, chessboard_size, stereo_img);
    addHelpText(view);
    addNumberOfImagesText(view, stereo_images.size());

    cv::imshow(win_name, view);

    const auto key = static_cast<char>(cv::waitKey(10));
    if (key == ' ')
    {
      stereo_images.push_back(stereo_img);
    }
    else if (key == 'q')
    {
      break;
    }
  }

  return std::move(stereo_images);
}
