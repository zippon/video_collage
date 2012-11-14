//
//  video_saliency.cpp
//  video_collage
//
//  Created by Zhipeng Wu on 11/14/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#include "video_saliency.h"

const float kShotThreshold = 20.0;

bool VideoSaliency::Detect(const int expect_shot_num) {
  cv::VideoCapture video_capture;
  if (!video_capture.open(video_path_))
    return false;
  
  // Traverse the video and inser candidate shots into queue.
  SHOTCANDIDATE shot_candidates;
  
  // reset current start position to the begining
  video_capture.set(CV_CAP_PROP_POS_MSEC, 0);
  // we extract one frame per second.
  const int kFrameSkip = video_capture.get(CV_CAP_PROP_FPS) * 5;
  
  cv::Mat pre2, pre1, cur, next1, next2, temp;
  // set the start information for first shot
  for (int i = 0; i < 5; ++i) {
    video_capture >> temp;
  }
  // Not color image?
  if (3 != temp.channels())
    return false;
  //cv::cvtColor(cur, cur, CV_RGB2GRAY);
  frame_width_ = temp.cols;
  frame_height_ = temp.rows;
  float cur_time;
  
  // frame_counter: counting and extract frames based on the step
  int frame_counter = 1;
  while (video_capture.grab()) {
    ++frame_counter;
    
    if (!(frame_counter % kFrameSkip)) {
      if (!video_capture.read(temp))  break;
      temp.copyTo(pre2);
      ++frame_counter;
      if (!video_capture.read(temp))  break;
      temp.copyTo(pre1);
      ++frame_counter;
      if (!video_capture.read(temp))  break;
      ++frame_counter;
      temp.copyTo(cur);
      if (!video_capture.read(temp))  break;
      temp.copyTo(next1);
      ++frame_counter;
      if (!video_capture.read(temp))  break;
      temp.copyTo(next2);
      ++frame_counter;
      
      cv::cvtColor(pre2, pre2, CV_RGB2GRAY);
      cv::cvtColor(pre1, pre1, CV_RGB2GRAY);
      cv::cvtColor(cur, cur, CV_RGB2GRAY);
      cv::cvtColor(next1, next1, CV_RGB2GRAY);
      cv::cvtColor(next2, next2, CV_RGB2GRAY);

      CvScalar diff = cv::sum(cv::abs(pre2 - pre1) + cv::abs(pre1 - cur) +
                              cv::abs(cur - next1) + cv::abs(next1 - next2));
      float score = diff.val[0] / (frame_height_ * frame_width_);
                                    
      if (score > kShotThreshold) {
        cout << "frame_diff @ " << frame_counter << " : " << score << endl;
        cur_time = video_capture.get(CV_CAP_PROP_POS_MSEC) / 1000;
        SHOT shot(score, cur_time);
        shot_candidates.push(shot);
      }
    }
  }
  
  // Select the top k shot detection score (frame_diff) as the out put.
  shot_num_ = (shot_candidates.size() > expect_shot_num) ? expect_shot_num :
  static_cast<int>(shot_candidates.size());
  for (int i = 0; i < shot_num_; ++i) {
    shots_.push_back(shot_candidates.top());
    shot_candidates.pop();
  }
  return true;
}