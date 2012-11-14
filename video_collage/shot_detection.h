//
//  shot_detection.h
//  video_collage
//
//  Created by Zhipeng Wu on 11/11/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#ifndef __video_collage__shot_detection__
#define __video_collage__shot_detection__

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;
using std::priority_queue;
using std::sort;

typedef pair<float, float> SHOT;
struct ShotCmp {
  bool operator() (const SHOT& s1, const SHOT& s2) {
    return s1.first > s2.first;
  }
};
struct TimeCmp {
  bool operator() (const SHOT& s1, const SHOT& s2) {
    return s1.second < s2.second;
  }
};
typedef priority_queue<SHOT, vector<SHOT>, ShotCmp> SHOTCANDIDATE;
typedef vector<SHOT> SHOTQUEUE;

class ShotDetection {
public:
  // Constructors:
  explicit ShotDetection(const string& path): video_path_(path), shot_num_(0) {
  }
  // Workers:
  bool Detect(const int expect_shot_num);
  // Accessors:
  int shot_num() const {
    return shot_num_;
  }
  const SHOTQUEUE shots() const {
    return shots_;
  }
  const int frame_width() const {
    return frame_width_;
  }
  const int frame_height() const {
    return frame_height_;
  }
  void PrintDetectedShots() const {
    if (shot_num_) {
      cout << "Detected " << shot_num_ << " shots.\n";
      for (const auto& shot : shots_) {
        cout << "Shot score: " << shot.first;
        cout << "\t\tShot start: " << shot.second << endl;
      }
    } else {
      cout << "No detected shots\n";
    }
  }
  
private:
  
  // Compare frame difference based on color histogram.
  float CompareFrameDiffRGB(const cv::Mat& f1, const cv::Mat& f2) const;
  float CompareFrameDiffHSV(const cv::Mat& f1, const cv::Mat& f2) const;
  int shot_num_;
  string video_path_;
  SHOTQUEUE shots_;
  int frame_width_;
  int frame_height_;
  
  // Disallow copy and assign.
  ShotDetection& operator= (const ShotDetection&);
  ShotDetection(const ShotDetection&);
};

#endif /* defined(__video_collage__shot_detection__) */
