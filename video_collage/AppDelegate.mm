//
//  AppDelegate.m
//  video_collage
//
//  Created by Zhipeng Wu on 11/11/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//


#import "AppDelegate.h"

// openFiles is a simple C function that opens an NSOpenPanel and return an array of URLs
static NSString *openFiles()
{
  NSOpenPanel *panel;
  NSArray* fileTypes = [[NSArray alloc] initWithObjects:@"mp4", @"MP4", @"webm", @"ogg", nil];
  panel = [NSOpenPanel openPanel];
  [panel setFloatingPanel:YES];
  [panel setCanChooseDirectories:NO];
  [panel setAllowsMultipleSelection:NO];
  [panel setCanChooseFiles:YES];
  [panel setAllowedFileTypes:fileTypes];
	NSInteger i = [panel runModal];
	if (i == NSOKButton)
  {
    return [[panel filenames] objectAtIndex:0];
  }
  
  return nil;
}

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  tile_num_ = 20;
  video_path_ = "";
}

- (IBAction)LoadFile:(id)sender {
  NSString *local_path = openFiles();
  
  if (!local_path)
  {
    NSLog(@"No files selected, return...");
    return;
  }

  video_path_ = [local_path UTF8String];
  [self.video_path setStringValue:local_path];
}

- (IBAction)SetTileNum:(id)sender {
  
  [self.tile_num setIntegerValue:[sender integerValue]];
  tile_num_ = static_cast<int>([sender integerValue]);
}

- (IBAction)ShotCollage:(id)sender {
  if ("" != video_path_) {
    // Step 1: Shot detection.
    boost::scoped_ptr<ShotDetection> detector(new ShotDetection(video_path_));
    if (!detector->Detect(tile_num_)) {
      NSAlert *alert= [NSAlert alertWithMessageText:@"Shot Detection Failed."
          defaultButton:@"OK" alternateButton:@"Cancel" otherButton:nil
          informativeTextWithFormat:@"Shot Detection Failed."];
      if ([alert runModal]!=NSAlertDefaultReturn){
        NSLog(@"cancel");
      }
      else{
        NSLog(@"ok");
      }
    }

    // Step 2: Collage generation.
    float tile_alpha = static_cast<float>(detector->frame_width()) /
        detector->frame_height();
    tile_num_ = detector->shot_num();
    boost::scoped_ptr<VideoCollage> collage(new VideoCollage(tile_num_, tile_alpha));
    if(!collage->CreateCollage(1000, 1.5, 1.05)) {
      NSAlert *alert= [NSAlert alertWithMessageText:@"Collage Generation Failed." defaultButton:@"OK" alternateButton:@"Cancel" otherButton:nil
          informativeTextWithFormat:@"Collage Generation Failed."];
      if ([alert runModal]!=NSAlertDefaultReturn){
        NSLog(@"cancel");
      }
      else{
        NSLog(@"ok");
      }
    }
    
    // Step 3: Html output.
    if (!OutputHtml(detector->shots(), collage->tile_array_pos(),
        video_path_, "/tmp/video_collage.html")) {
      NSAlert *alert= [NSAlert alertWithMessageText:@"Html Output Failed."
          defaultButton:@"OK" alternateButton:@"Cancel" otherButton:nil
          informativeTextWithFormat:@"Html Output Failed."];
      if ([alert runModal]!=NSAlertDefaultReturn){
        NSLog(@"cancel");
      }
      else{
        NSLog(@"ok");
      }
    }
    [[NSWorkspace sharedWorkspace] openURL:[NSURL
        URLWithString:@"file://localhost/private/tmp/video_collage.html"]];
  }
}

- (IBAction)SaliencyCollage:(id)sender {
}
@end
