//
//  KBSymbolsView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-11.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "KeyboardView.h"

@interface KBSymbolsView : KeyboardView

- (void)initialize;
- (IBAction)buttonPressed:(id)sender;
- (IBAction)buttonLongPressed:(id)sender;

@end
