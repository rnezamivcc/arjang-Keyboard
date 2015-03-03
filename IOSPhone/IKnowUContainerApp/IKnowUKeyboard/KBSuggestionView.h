//
//  KBSuggestionView.h
//  IKnowUContainerApp
//
//  Created by Chris Bateman on 2014-07-16.
//  Copyright (c) 2014 WordLogic Corporation. All rights reserved.
//

#import "KBPhraseButton.h"
#import "KBPredButton.h"
#import "KeyboardView.h"

@interface KBSuggestionView : KeyboardView

@property (strong, nonatomic) IBOutlet UIScrollView *scrollView;
@property (strong, nonatomic) IBOutlet UIView *contentView;

@property (strong, nonatomic) IBOutlet KBPhraseButton *phraseOneButton;
@property (strong, nonatomic) IBOutlet KBPhraseButton *phraseTwoButton;
@property (strong, nonatomic) IBOutlet KBPhraseButton *phraseThreeButton;

@property (strong, nonatomic) IBOutlet KBPredButton *predOneButton;
@property (strong, nonatomic) IBOutlet KBPredButton *predTwoButton;
@property (strong, nonatomic) IBOutlet KBPredButton *predThreeButton;
@property (strong, nonatomic) IBOutlet KBPredButton *predFourButton;
@property (strong, nonatomic) IBOutlet KBPredButton *predFiveButton;

- (IBAction)buttonPressed:(id)sender;
- (IBAction)buttonLongPressed:(id)sender;

- (void)initialize;
- (void)clearSuggestions;
- (void)clearPhrases;
- (void)updateSuggestions:(NSArray *)suggestions;
- (void)updatePhrases:(NSArray *)phrases;

@end
