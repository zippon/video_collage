//
//  make_collage.cpp
//  video_collage
//
//  Created by Zhipeng Wu on 11/12/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#include "make_collage.h"

// Recursively calculate aspect ratio for all the inner nodes.
// The return value is the aspect ratio for the node.
float VideoCollage::CalculateAlpha(TreeNode* node) {
  if (!node->is_leaf_) {
    float left_alpha = CalculateAlpha(node->left_child_);
    float right_alpha = CalculateAlpha(node->right_child_);
    if (node->split_type_ == 'v') {
      node->alpha_ = left_alpha + right_alpha;
      return node->alpha_;
    } else if (node->split_type_ == 'h') {
      node->alpha_ = (left_alpha * right_alpha) / (left_alpha + right_alpha);
      return node->alpha_;
    } else {
      cout << "Error: CalculateAlpha" << endl;
      return -1;
    }
  } else {
    // This is a leaf node, just return the image's aspect ratio.
    return node->alpha_;
  }
}

// Top-down Calculate the image positions in the colage.
bool VideoCollage::CalculatePositions(TreeNode* node) {
  // Step 1: calculate height & width.
  if (node->parent_->split_type_ == 'v') {
    // Vertical cut, height unchanged.
    node->position_.height_ = node->parent_->position_.height_;
    if (node->child_type_ == 'l') {
      node->position_.width_ = node->position_.height_ * node->alpha_;
    } else if (node->child_type_ == 'r') {
      node->position_.width_ = node->parent_->position_.width_ -
      node->parent_->left_child_->position_.width_;
    } else {
      cout << "Error: CalculatePositions step 0" << endl;
      return false;
    }
  } else if (node->parent_->split_type_ == 'h') {
    // Horizontal cut, width unchanged.
    node->position_.width_ = node->parent_->position_.width_;
    if (node->child_type_ == 'l') {
      node->position_.height_ = node->position_.width_ / node->alpha_;
    } else if (node->child_type_ == 'r') {
      node->position_.height_ = node->parent_->position_.height_ -
      node->parent_->left_child_->position_.height_;
    }
  } else {
    cout << "Error: CalculatePositions step 1" << endl;
    return false;
  }
  
  // Step 2: calculate x & y.
  if (node->child_type_ == 'l') {
    // If it is left child, use its parent's x & y.
    node->position_.x_ = node->parent_->position_.x_;
    node->position_.y_ = node->parent_->position_.y_;
  } else if (node->child_type_ == 'r') {
    if (node->parent_->split_type_ == 'v') {
      // y (row) unchanged, x (colmn) changed.
      node->position_.y_ = node->parent_->position_.y_;
      node->position_.x_ = node->parent_->position_.x_ +
      node->parent_->position_.width_ -
      node->position_.width_;
    } else if (node->parent_->split_type_ == 'h') {
      // x (column) unchanged, y (row) changed.
      node->position_.x_ = node->parent_->position_.x_;
      node->position_.y_ = node->parent_->position_.y_ +
      node->parent_->position_.height_ -
      node->position_.height_;
    } else {
      cout << "Error: CalculatePositions step 2 - 1" << endl;
    }
  } else {
    cout << "Error: CalculatePositions step 2 - 2" << endl;
    return false;
  }
  
  // Calculation for children.
  if (node->left_child_) {
    bool success = CalculatePositions(node->left_child_);
    if (!success) return false;
  }
  if (node->right_child_) {
    bool success = CalculatePositions(node->right_child_);
    if (!success) return false;
  }
  return true;
}

bool VideoCollage::AdjustAlpha(TreeNode *node, const float thresh) {
  assert(thresh > 1);
  if (node->is_leaf_) return false;
  if (node == NULL) return false;
  
  bool changed = false;
  
  float thresh_2 = 1 + (thresh - 1) / 2;
  
  if (node->alpha_ > node->alpha_expect_ * thresh_2) {
    // Too big actual aspect ratio.
    if (node->split_type_ == 'v') changed = true;
    node->split_type_ = 'h';
    node->left_child_->alpha_expect_ = node->alpha_expect_ * 2;
    node->right_child_->alpha_expect_ = node->alpha_expect_ * 2;
  } else if (node->alpha_ < node->alpha_expect_ / thresh_2 ) {
    // Too small actual aspect ratio.
    if (node->split_type_ == 'h') changed = true;
    node->split_type_ = 'v';
    node->left_child_->alpha_expect_ = node->alpha_expect_ / 2;
    node->right_child_->alpha_expect_ = node->alpha_expect_ / 2;
  } else {
    // Aspect ratio is okay.
    if (node->split_type_ == 'h') {
      node->left_child_->alpha_expect_ = node->alpha_expect_ * 2;
      node->right_child_->alpha_expect_ = node->alpha_expect_ * 2;
    } else if (node->split_type_ == 'v') {
      node->left_child_->alpha_expect_ = node->alpha_expect_ / 2;
      node->right_child_->alpha_expect_ = node->alpha_expect_ */2;
    } else {
      cout << "Error: AdjustAlpha" << endl;
      return false;
    }
  }
  bool changed_l = AdjustAlpha(node->left_child_, thresh);
  bool changed_r = AdjustAlpha(node->right_child_, thresh);
  return changed||changed_l||changed_r;
}

void VideoCollage::RandomSplitType(TreeNode* node) {
  if (node == NULL) return;
  if (node->is_leaf_ == true) return;
  int v_h = random(2);
  if (v_h == 1) {
    node->split_type_ = 'v';
  } else if (v_h == 0) {
    node->split_type_ = 'h';
  } else {
    cout << "Error: RandomSplitType()" << endl;
    return;
  }
  RandomSplitType(node->left_child_);
  RandomSplitType(node->right_child_);
}

// Generate abinary tree with tile_num_ leaves.
bool VideoCollage::GenerateBinaryTree(const float expect_alpha) {
  tree_leaves_.clear();
  if (tree_root_)  ReleaseTree(tree_root_);
  tree_root_ = new TreeNode();
  tree_root_->alpha_expect_ = expect_alpha;
  // Step 1: create a (k-1)-depth binary tree with max nodes.
  // 2 ^ (k - 1) <= m < 2 ^ k
  int m = tile_num_;
  assert(m != 0);
  int k = 0;
  while (m != 0) {
    m >>= 1;
    ++k;
  }
  vector<vector<TreeNode*>> node_queue;
  for (int i = 0; i < k; ++i) {
    std::vector<TreeNode*> node_queue_temp;
    node_queue.push_back(node_queue_temp);
  }
  node_queue[0].push_back(tree_root_);
  for (int i = 0; i < k - 1; ++i) {
    for (int j = 0; j < pow(2, i); ++j) {
      
      node_queue[i][j]->left_child_ = new TreeNode();
      node_queue[i][j]->left_child_->child_type_ = 'l';
      node_queue[i][j]->left_child_->parent_ = node_queue[i][j];
      node_queue[i + 1].push_back(node_queue[i][j]->left_child_);
      
      node_queue[i][j]->right_child_ = new TreeNode();
      node_queue[i][j]->right_child_->child_type_ = 'r';
      node_queue[i][j]->right_child_->parent_ = node_queue[i][j];
      node_queue[i + 1].push_back(node_queue[i][j]->right_child_);
      
      node_queue[i][j]->is_leaf_ = false;
    }
  }
  vector<TreeNode*> leaf_nodes;
  for (int i = 0; i < node_queue[k - 1].size(); ++i) {
    leaf_nodes.push_back(node_queue[k - 1][i]);
  };
  
  int leaf_num = pow(2, k - 1);
  bool* leaf_visited = new bool[leaf_num];
  for (int i = 0; i < leaf_num; ++i) leaf_visited[i] = false;
  assert(static_cast<int>(leaf_nodes.size()) == leaf_num);
  // Step 2: randomly select image_num_ - 2 ^ (k - 1) leaves,
  // split them with left and right children. Then, you have a
  // full balanced binary tree with image_num_ leaves.
  
  int left_leaves = tile_num_ - leaf_num;
  int counter = 0;
  while (counter < left_leaves) {
    int rand_ind = random(left_leaves);
    if (leaf_visited[rand_ind] == true) continue;
    leaf_visited[rand_ind] = true;
    leaf_nodes[rand_ind]->is_leaf_ = false;
    leaf_nodes[rand_ind]->left_child_ = new TreeNode();
    leaf_nodes[rand_ind]->left_child_->child_type_ = 'l';
    leaf_nodes[rand_ind]->left_child_->parent_ = leaf_nodes[rand_ind];
    leaf_nodes[rand_ind]->right_child_ = new TreeNode();
    leaf_nodes[rand_ind]->right_child_->child_type_ = 'r';
    leaf_nodes[rand_ind]->right_child_->parent_ = leaf_nodes[rand_ind];
    tree_leaves_.push_back(leaf_nodes[rand_ind]->left_child_);
    tree_leaves_.push_back(leaf_nodes[rand_ind]->right_child_);
    ++counter;
  }
  for (int i = 0; i < leaf_num; ++i) {
    if (leaf_visited[i] != true) tree_leaves_.push_back(leaf_nodes[i]);
  }
  // Now we have created a binary tree with tile_num_ leaves.
  // And the vector leaf_nodes_new stores all the leaf nodes.
  assert(static_cast<int>(tree_leaves_.size()) == tile_num_);
  delete [] leaf_visited;
  leaf_nodes.clear();
  
  // Step 3: set alpha to leaf nodes.
  for (auto& leaf: tree_leaves_) {
    leaf->alpha_ = tile_alpha_;
  }
  // Step 4: assign a random 'v' or 'h' for all the inner nodes.
  RandomSplitType(tree_root_);
  return true;
}

// If we use CreateCollage, the generated collage may have strange aspect ratio such as
// too big or too small, which seems to be difficult to be shown. We let the user to
// input their expected aspect ratio and fast adjust to make the result aspect ratio
// close to the user defined one.
// The thresh here controls the closeness between the result aspect ratio and the expect
// aspect ratio. e.g. expect_alpha is 1, thresh is 2. The result aspect ratio is around
// [1 / 2, 1 * 2] = [0.5, 2].
// We also define MAX_ITER_NUM = 100,
// If max iteration number is reached and we cannot find a good result aspect ratio,
// this function returns false.
bool VideoCollage::CreateCollage(const int canvas_width,
                                const float expect_alpha,
                                const float thresh) {
  assert(thresh > 1);
  assert(expect_alpha > 0);
  canvas_width_ = canvas_width;
  
  float lower_bound = expect_alpha / thresh;
  float upper_bound = expect_alpha * thresh;
  int total_iter_counter = 1;
  int iter_counter = 1;
  int tree_gene_counter = 1;
  
  // Do the initial tree generatio and calculation.
  // A: generate a full balanced binary tree with image_num_ leaves.
  GenerateBinaryTree(expect_alpha);
  // B: recursively calculate aspect ratio.
  canvas_alpha_ = CalculateAlpha(tree_root_);
  
  while ((canvas_alpha_ < lower_bound) || (canvas_alpha_ > upper_bound)) {
    bool changed = false;
    changed = AdjustAlpha(tree_root_, thresh);
    canvas_alpha_ = CalculateAlpha(tree_root_);
    ++total_iter_counter;
    ++iter_counter;
    if ((iter_counter > kMaxIterNum) || (!changed)) {
      /*cout << "********************************************" << endl;
      if (changed) {
        cout << "max iteration number reached..." << endl;
      } else {
        cout << "tree structure unchanged after iteration: "
            << iter_counter << endl;
      }
      cout << "********************************************" << endl;*/
      // We should generate binary tree again
      iter_counter = 1;
      ++total_iter_counter;
      /*************************************************************************/
      
      GenerateBinaryTree(expect_alpha);
      canvas_alpha_ = CalculateAlpha(tree_root_);
      ++tree_gene_counter;
      if (tree_gene_counter > kMaxGeneNum) {
        cout << "-------------------------------------------------------";
        cout << endl;
        cout << "WE HAVE DONE OUR BEST, BUT COLLAGE GENERATION FAILED...";
        cout << endl;
        cout << "-------------------------------------------------------";
        cout << endl;
        return false;
      }
    }
  }
  
  cout << "Total iteration number is: " << total_iter_counter << endl;
  // After adjustment, set the position for all the tile images.
  canvas_height_ = static_cast<int>(canvas_width_ / canvas_alpha_);
  tree_root_->position_.x_ = 0;
  tree_root_->position_.y_ = 0;
  tree_root_->position_.height_ = canvas_height_;
  tree_root_->position_.width_ = canvas_width_;
  if (tree_root_->left_child_)
    CalculatePositions(tree_root_->left_child_);
  if (tree_root_->right_child_)
    CalculatePositions(tree_root_->right_child_);
  TreePosCmp tree_pos_cmp;
  sort(tree_leaves_.begin(), tree_leaves_.end(), tree_pos_cmp);
  for (const auto& leaf: tree_leaves_) {
    FloatRect rect(leaf->position_.x_, leaf->position_.y_,
                   leaf->position_.width_, leaf->position_.height_);
    tile_array_pos_.push_back(rect);
  }
  TreeSizeCmp tree_size_cmp;
  sort(tree_leaves_.begin(), tree_leaves_.end(), tree_size_cmp);
  for (const auto& leaf: tree_leaves_) {
    FloatRect rect(leaf->position_.x_, leaf->position_.y_,
                   leaf->position_.width_, leaf->position_.height_);
    tile_array_size_.push_back(rect);
  }
  cout << "Canvas alpha:\t" << canvas_alpha_ << endl;
  return true;
}

bool OutputHtml(const SHOTQUEUE& shots, const vector<FloatRect>& tiles,
                const string& video_path, const string& save_html) {
  assert(0 != shots.size());
  assert(shots.size() == tiles.size());
  ofstream output_html(save_html.c_str());
  if (!output_html) {
    cout << "Error: OutputCollageHtml" << endl;
    return false;
  }
  
  output_html << "<!DOCTYPE html><html><head><SCRIPT LANGUAGE=\"javascript\">fu\
nction openwin(skip, video_path){\n";
  output_html << "OpenWindow=window.open(\"video\",\"channelmode\
=yes,fullscreen=yes, left=0, top=0, titlebar=no, toolbar=no, location=no, direc\
tories=no, status=no, menubar=no, scrollbars=no, resizable=yes, copyhistory=no\
\")\n";
  output_html << "OpenWindow.document.write(\"<BODY BGCOLOR=#000000>\")\n";
  output_html << "OpenWindow.document.write(\"<table width='100%' height='100%'\
><tr><td align='center' valign='middle'><video autoplay controls ondblclick='wi\
ndow.close()' onloadeddata='this.currentTime=\"+skip+\"'><source src='\"+video_\
path+\"'/></video></td><tr></BODY>\")}\n</script\></head><style type=\"text/css\"> \
body {background-image:url(/Users/WU/Projects/2012Collage/JMM_Demo/video_collag\
e_bg.jpg);  background-position: top; background-repeat:repeat-x; background-attac\
hment:fixed}</style><body><div style=\"mar\
gin:20px auto; width:72%; position:relative;\">";
  for (int i = 0; i < shots.size(); ++i) {
    output_html << "<video ondblclick=\"openwin(";
    output_html << shots[i].second << ",";
    output_html << "'" << video_path << "')\"";
    output_html << " onloadeddata=\"this.currentTime=";
    output_html << shots[i].second << "\" style=\"position:absolute;";
    output_html << "width:" << tiles[i].width_ << "px; height:"
        << tiles[i].height_ << "px; left:"
        << tiles[i].x_ << "px; top:"
        << tiles[i].y_ << "px;\"><source src=\"";
    output_html << video_path;
    output_html << "\" /></video>";
  }
  output_html << "</div></body></html>";
  output_html.close();
  return true;
}


