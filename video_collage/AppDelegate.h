//
//  AppDelegate.h
//  video_collage
//
//  Created by Zhipeng Wu on 11/11/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#import "shot_detection.h"
#import "make_collage.h"
#import "video_saliency.h"
#import <boost/scoped_ptr.hpp>
#import <Quartz/Quartz.h>
#import <Cocoa/Cocoa.h>
#import <string>
#import <vector>
#import <iostream>

@interface AppDelegate : NSObject <NSApplicationDelegate> {
  std::string video_path_;
  int tile_num_;
}
@property (weak) IBOutlet NSTextField *video_path;
@property (weak) IBOutlet NSTextField *tile_num;
@property (assign) IBOutlet NSWindow *window;

- (IBAction)LoadFile:(id)sender;
- (IBAction)SetTileNum:(id)sender;
- (IBAction)ShotCollage:(id)sender;
- (IBAction)SaliencyCollage:(id)sender;
@end
