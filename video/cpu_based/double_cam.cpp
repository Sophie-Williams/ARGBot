#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "ovr.h"

// can be 30-45ish
#define DCAMFPS 30

using namespace arma;

int main() {
  // open both left and right cameras
  cv::VideoCapture left(0);
  cv::VideoCapture right(1);
  assert(left.isOpened() && right.isOpened());

  // set properties
  left.set(CV_CAP_PROP_FRAME_WIDTH, 640); 
  left.set(CV_CAP_PROP_FRAME_HEIGHT, 480); 
  left.set(CV_CAP_PROP_FPS, DCAMFPS); 
  right.set(CV_CAP_PROP_FRAME_WIDTH, 640); 
  right.set(CV_CAP_PROP_FRAME_HEIGHT, 480); 
  right.set(CV_CAP_PROP_FPS, DCAMFPS);

  // (optional) try to identify the different cameras
/*  double gain1 = left.get(CV_CAP_PROP_GAIN);
  double gain2 = right.get(CV_CAP_PROP_GAIN);
  bool swapped = false;
  if (gain1 > 0.6 && gain2 < 0.6) {
    swapped = true;
  }
*/
  // grab and display the frame
  cv::namedWindow("hud");
  cv::Mat frames[2];
  double offset = 0.15;
  cube limg, rimg, combined;

  for (;;) {
    left.read(frames[0]);
    right.read(frames[1]);
    if (!frames[0].data || !frames[1].data) {
      printf("No data...\n");
      continue;
    }
    // statically mapped numbers - assumption is that the frame is going to be 480x640 big
    /*if (swapped) {
      limg = cvt_opencv2arma(frames[1]) / 255.0;
      rimg = cvt_opencv2arma(frames[0]) / 255.0;
    } else {*/
      limg = cvt_opencv2arma(frames[0]) / 255.0;
      rimg = cvt_opencv2arma(frames[1]) / 255.0;
//    }
    limg = limg(span(0, 479), span(128, 510), span::all);
    rimg = rimg(span(0, 479), span(128, 510), span::all);
    combined = ovr_image(limg, rimg, offset); // waste copy
    disp_image("hud", combined);
    if (disp_keyPressed() >= 0) {
      break;
    }
  }
  return 0;
}
