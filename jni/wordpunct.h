//////////////////////////////////////////////////////////////////////////////////////////////
// Used by both autocorrect and prediction engine.
// This file contains support for generic punctuation calls, so DO NOT make any changes which  make it dependent on specific calls
// in jni classes such as compactstore.*, compactDict, dictManager.* and so on.
//////////////////////////////////////////////////////////////////////////////////////////////
#ifndef WORDPUNCT_H
#define WORDPUNCT_H

#include "dictrun.h"

CPPExternOpen
bool wordIsInBrackets(WCHAR *word);
bool wordEndsOnNormalPunctuation(MYWCHAR endofwordchar);
int declutterWord(MYWCHAR *text, int nValidNodes);

PuncChoice *findPunctuationChoiceAtEndOfWord(MYWCHAR endofwordchar);
PuncChoice *findPunctuationChoice(MYWCHAR letter);
bool isLetterOrNumber(MYWCHAR c);
bool wordIsANumber(MYWCHAR *word, int nPunctChars);

int containsSPs(MYWCHAR *text);
inline BOOL isEndOfSentencePunctuation(MYWCHAR c)
{
	return (c == (MYWCHAR)'.' || c == (MYWCHAR)'?' || c == (MYWCHAR)'!' || c == (MYWCHAR)';' || c==CR || c==LF);
}
bool isEndOfWordPunctuation(MYWCHAR c);

CPPExternClose

#endif