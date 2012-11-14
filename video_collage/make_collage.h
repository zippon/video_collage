//
//  make_collage.h
//  video_collage
//
//  Created by Zhipeng Wu on 11/12/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#ifndef __video_collage__make_collage__
#define __video_collage__make_collage__

#include "shot_detection.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <time.h>
#include <algorithm>
#include <fstream>
#define random(x) (rand() % x)

const int kMaxIterNum = 100;
const int kMaxGeneNum = 100;

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::sort;
using std::ofstream;

class FloatRect {
public:
  FloatRect() : x_(0), y_(0), width_(0), height_(0) { }
  FloatRect(float x, float y, float w, float h) :
      x_(x), y_(y), width_(w), height_(h) { }
  float x_;
  float y_;
  float width_;
  float height_;
};

class TreeNode {
public:
  TreeNode() {
    child_type_ = 'N';
    split_type_ = 'N';
    is_leaf_ = true;
    alpha_ = 0.0;
    alpha_expect_ = 0.0;
    position_ = FloatRect();
    left_child_ = NULL;
    right_child_ = NULL;
    parent_ = NULL;
  }
  char child_type_;      // Is this node left child "l" or right child "r".
  char split_type_;      // If this node is a inner node, we set 'v' or 'h', which indicate
  // vertical cut or horizontal cut.
  bool is_leaf_;         // Is this node a leaf node or a inner node.
  float alpha_expect_;   // If this node is a leaf, we set expected aspect ratio of this node.
  float alpha_;          // If this node is a leaf, we set actual aspect ratio of this node.
  FloatRect position_;    // The position of the node on canvas.
  TreeNode* left_child_;
  TreeNode* right_child_;
  TreeNode* parent_;
};

struct TreePosCmp {
  bool operator() (TreeNode* n1, TreeNode* n2) {
    assert(NULL != n1);
    assert(NULL != n2);
    if (n1->position_.y_ < n2->position_.y_) {
      return true;
    } else if (n1->position_.y_ == n2->position_.y_) {
      if (n1->position_.x_ < n2->position_.x_) {
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
};

struct TreeSizeCmp {
  bool operator() (TreeNode* n1, TreeNode* n2) {
    assert(NULL != n1);
    assert(NULL != n2);
    float size1 = n1->position_.width_ * n1->position_.height_;
    float size2 = n2->position_.width_ * n2->position_.height_;
    return size1 > size2;
  }
};

class VideoCollage {
public:
  // Constructors.
  VideoCollage(const int tile_num, const float tile_alpha) : tile_num_(tile_num),
      tile_alpha_(tile_alpha), canvas_width_(-1), canvas_alpha_(-1),
      canvas_height_(-1) {
    srand(static_cast<unsigned>(time(0)));
    tree_root_ = new TreeNode();
  }
  ~VideoCollage() {
    ReleaseTree(tree_root_);
  }
  
  // Public member functions.
  bool CreateCollage(const int canvas_width, const float expect_alpha,
                     const float thresh);
  // Accessors:
  const int tile_num() const {
    return tile_num_;
  }
  const int canvas_height() const {
    return canvas_height_;
  }
  const int canvas_width() const {
    return canvas_width_;
  }
  const float canvas_alpha() const {
    return canvas_alpha_;
  }
  const vector<FloatRect>& tile_array_pos() {
    return tile_array_pos_;
  }
  const vector<FloatRect>& tile_array_size() {
    return tile_array_size_;
  }

private:
  // Private member functions.
  void ReleaseTree(TreeNode* node) {
    if (NULL == node)  return;
    ReleaseTree(node->left_child_);
    ReleaseTree(node->right_child_);
  }
  // Recursively calculate aspect ratio for all the inner nodes.
  float CalculateAlpha(TreeNode* node);
  // Top-down Calculate the image positions in the colage.
  bool CalculatePositions(TreeNode* node);
  // Top-down adjust aspect ratio for the final collage.
  bool AdjustAlpha(TreeNode* node, const float thresh);
  // Random assign a 'v' (vertical cut) or 'h' (horizontal cut) for all the inner nodes.
  void RandomSplitType(TreeNode* node);
  // Generate a balanced binary tree with tile_num_ leaf nodes.
  bool GenerateBinaryTree(const float expect_alpha);
  
  
  // Private member variables.
  int tile_num_;
  float tile_alpha_;
  int canvas_width_;
  int canvas_height_;
  float canvas_alpha_;
  TreeNode* tree_root_;
  vector<TreeNode*> tree_leaves_;
  vector<FloatRect> tile_array_pos_;
  vector<FloatRect> tile_array_size_;
};

bool OutputHtml(const SHOTQUEUE& shots, const vector<FloatRect>& tiles,
                const string& video_path, const string& save_html);

#endif /* defined(__video_collage__make_collage__) */
