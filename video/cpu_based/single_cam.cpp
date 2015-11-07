#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <armadillo>
#include "highgui.h"
#include "ovr.h"

#define DCAMFPS 30

using namespace arma;

int main() {
  cv::VideoCapture cam(0);
  cam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
  cam.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
  cv::namedWindow("hud");

  cv::Mat frame;
  double offset = 0.15;
  cube limg, rimg, combined;

  for (;;) {
    cam.read(frame);
    if (!frame.data) {
      printf("No data...\n");
      continue;
    }
    limg = cvt_opencv2arma(frame) / 255.0;
    rimg = cvt_opencv2arma(frame) / 255.0;
    limg = limg(span(0, 479), span(108, 490), span::all);
    rimg = rimg(span(0, 479), span(148, 530), span::all);
    // this is where we combine the images
    combined = ovr_image(limg, rimg, offset);
    disp_image("hud", combined);
    if (disp_keyPressed() >= 0) {
      break;
    }
  }
  return 0;
}
