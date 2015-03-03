//
//  MiniAppBarView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-08-08.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MiniAppBase.h"

@interface MiniAppBarView : UIView

- (void)clearAllMiniApps;
- (void)addMiniApp:(MiniAppBase *)miniApp;

@end
