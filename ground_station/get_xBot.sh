#!/bin/bash
gst-launch-0.10 -v udpsrc port=9000 ! \
  multifilesink post-messages=true location="frame%d.jpg" max-files=100
#    jpegdec ! \
#    autovideosink
