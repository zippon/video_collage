//
//  video_saliency.h
//  video_collage
//
//  Created by Zhipeng Wu on 11/14/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#ifndef __video_collage__video_saliency__
#define __video_collage__video_saliency__

#include "shot_detection.h"
    
class VideoSaliency {
public:
  // Constructors:
  explicit VideoSaliency(const string& path): video_path_(path), shot_num_(0) { }
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
      

  int shot_num_;
  string video_path_;
  SHOTQUEUE shots_;
  int frame_width_;
  int frame_height_;
      
  // Disallow copy and assign.
  VideoSaliency& operator= (const VideoSaliency&);
  VideoSaliency(const VideoSaliency&);
};

#endif /* defined(__video_collage__video_saliency__) */
