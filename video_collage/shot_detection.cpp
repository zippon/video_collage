//
//  shot_detection.cpp
//  video_collage
//
//  Created by Zhipeng Wu on 11/11/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#include "shot_detection.h"

// Shot detection threshold.
const float kShotThreshold = 0.5;

float ShotDetection::CompareFrameDiffRGB(const cv::Mat& f1, const cv::Mat& f2) const {
  // color histograms for input images
  cv::MatND f1_hist, f2_hist;
  
  // parameters for calculating color histogram
  // 8-bin for B,G,R
  int histSize[3] = {8, 8, 8};
  // pixel value range
  float hranges[2] = {0.0, 255.0};
  // three channels
  int channels[3] = {0, 1, 2};
  const float* ranges[3];
  // all channels have the same range
  ranges[0] = hranges;
  ranges[1] = hranges;
  ranges[2] = hranges;
  
  // calculate histogram based on opencv function
  calcHist(&f1,
           1,                 // histogram of f1 image only
           channels,          // three channels
           cv::Mat(),         // no masks
           f1_hist,           // output histogram
           3,                 // 3D histogram
           histSize,          // number of bins: 8
           ranges);           // pixel value 0.0 - 255.0
  
  calcHist(&f2, 1, channels, cv::Mat(), f2_hist, 3, histSize, ranges);
  
  float hist_sec = 0;
  int image_pixel = f1.rows * f1.cols;
  for (int iB = 0; iB < 8; ++iB) {
    for (int iG = 0; iG < 8; ++iG) {
      for (int iR = 0; iR < 8; ++iR) {
        hist_sec += std::min(f1_hist.at<float>(iB, iG, iR)
            ,f2_hist.at<float>(iB, iG, iR));
      }
    }
  }
  float ret = hist_sec / image_pixel;
  return 1 - ret;
}

float ShotDetection::CompareFrameDiffHSV(const cv::Mat& f1, const cv::Mat& f2) const {
  
  cv::Mat hsv_1, hsv_2;
  cv::cvtColor(f1, hsv_1, cv::COLOR_BGR2HSV);
  cv::cvtColor(f2, hsv_2, cv::COLOR_BGR2HSV);

  // color histograms for input images
  cv::MatND f1_hist, f2_hist;
  
  // parameters for calculating HSV color histogram
  // Quantize the hue to 20 levels
  // and the saturation to 32 levels
  int hbins = 20, sbins = 32;
  int histSize[] = {hbins, sbins};
  // hue varies from 0 to 179, see cvtColor
  float hranges[] = { 0, 180 };
  // saturation varies from 0 (black-gray-white) to
  // 255 (pure spectrum color)
  float sranges[] = { 0, 256};
  const float*ranges[] = { hranges, sranges };
  // we compute the histogram from the 0-th and 1-st channels
  int channels[] = {0, 1};
  
  // calculate histogram based on opencv function
  calcHist(&f1,
           1,                 // histogram of f1 image only
           channels,          // three channels
           cv::Mat(),         // no masks
           f1_hist,           // output histogram
           2,                 // 3D histogram
           histSize,          // number of bins: 8
           ranges);           // pixel value 0.0 - 255.0
  
  calcHist(&f2, 1, channels, cv::Mat(), f2_hist, 2, histSize, ranges);
  
  float hist_sec = 0;
  int image_pixel = f1.rows * f1.cols;
  for (int h = 0; h < hbins; ++h) {
    for (int s = 0; s < sbins; ++s) {
      hist_sec += std::min(f1_hist.at<float>(h, s) ,f2_hist.at<float>(h, s));
    }
  }
  float ret = hist_sec / image_pixel;
  return 1 - ret;
}


bool ShotDetection::Detect(const int expect_shot_num) {
  cv::VideoCapture video_capture;
  if (!video_capture.open(video_path_))
    return false;
  
  // Traverse the video and inser candidate shots into queue.
  SHOTCANDIDATE shot_candidates;
  
  // reset current start position to the begining
  video_capture.set(CV_CAP_PROP_POS_MSEC, 0);
  // we extract one frame per second.
  const int kFrameSkip = video_capture.get(CV_CAP_PROP_FPS);
  
  cv::Mat begin_image, cur_image, pre_image;
  // begin_image: the first frame of the shot
  // cur_image: current frame
  // pre_image: previous frame
  // begin_time: the start timestamp of the shot
  // shot gap is detected based on the different of cur_image and pre_image
  float begin_time;
  
  // set the start information for first shot
  for (int i = 0; i < 5; ++i) {
    video_capture >> cur_image;
  }
  // Not color image?
  if (3 != cur_image.channels())
    return false;
  frame_width_ = cur_image.cols;
  frame_height_ = cur_image.rows;
  
  // some video's first frame is black frame.
  cur_image.copyTo(begin_image);
  // Opencv's bug, we cannot use video_capture_ >> begin_image here
  // but use ur_image.copyTo(begin_image);
  
  begin_image.copyTo(pre_image);
  begin_time = video_capture.get(CV_CAP_PROP_POS_MSEC) / 1000;
  
  // frame_counter: counting and extract frames based on the step
  int frame_counter = 1;
  while (video_capture.grab()) {
    ++frame_counter;
    
    // A: extract frames and shot detect based on STEP (default = 5)
    if (!(frame_counter % kFrameSkip)) {
      video_capture.retrieve(cur_image);
      float frame_diff = CompareFrameDiffHSV(cur_image, pre_image);
      if (frame_diff > kShotThreshold) {
        begin_time = video_capture.get(CV_CAP_PROP_POS_MSEC) / 1000;
        SHOT shot(frame_diff, begin_time);
        shot_candidates.push(shot);
      }
      cur_image.copyTo(pre_image);
    }
  }
  
  // Select the top k shot detection score (frame_diff) as the out put.
  shot_num_ = (shot_candidates.size() > expect_shot_num) ? expect_shot_num :
      static_cast<int>(shot_candidates.size());
  for (int i = 0; i < shot_num_; ++i) {
    shots_.push_back(shot_candidates.top());
    shot_candidates.pop();
  }
  // Rank the elements in time acsending order.
  TimeCmp t_cmp;
  sort(shots_.begin(), shots_.end(), t_cmp);
  cout << "Shot number:\t" << shot_num_ << endl;
  return true;
}

