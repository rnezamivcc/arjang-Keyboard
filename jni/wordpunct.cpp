// <copyright file="wordpunc.cpp" company="WordLogic">
// Copyright (c) 2001, 2013 All Right Reserved, http://www.wordlogic.com/
//
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// </copyright>
// <author>Reza Nezami</author>
// <email>rnezami@wordlogic.com</email>
// <date>2012-06-10</date>
// <summary>This file provides set of utilities for word punctuation and ending and connecting words support.</summary>

#include "StdAfx.h"
#include "wordpunct.h"
#include <string.h>

#ifndef _WINDOWS
#include <ctype.h>
#include <stdlib.h>
#endif  // !WINDOWS

CPPExternOpen
//extern CDictManager *wpDictManager;


// space and return are not listed in the gPunctSymbols table
// as they have there own rules on implicit and explicit spacing
PuncChoice  gPunctSymbols[35] = 
{	// char  ,nSpaceAfter,nSpaceBefore,CanTerminate,CanBondWords  // both nSpacesAfter and nSpacesBefore are assumed to be 1,
    {	(MYWCHAR)'.',			1, 0,	TRUE,	TRUE  }
   ,{	(MYWCHAR)',',			1, 0,	TRUE,	FALSE }
   ,{	(MYWCHAR)'?',			1, 0,	TRUE,	FALSE }
   ,{	(MYWCHAR)'¿',			0, 1,	FALSE,	TRUE  }
   ,{	(MYWCHAR)'\"',		0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'\'',		0, 0,	FALSE,	TRUE  }		apostrophe  is not punctuation.
   ,{	(MYWCHAR)':',			0, 0,   TRUE,	FALSE }
   ,{	(MYWCHAR)';',			1, 0,	TRUE,	FALSE }
   ,{	(MYWCHAR)'!',			1, 0,	TRUE,	FALSE }
   //,{	(MYWCHAR)'|',			1, 0,	FALSE,	FALSE }
   //,{	(MYWCHAR)'&',			0, 1,	FALSE,	FALSE }
   //,{	(MYWCHAR)'_',			0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'-',			0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'+',			0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'/',			0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'\\',		0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'=',			0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'@',			0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'#',			0, 1,	FALSE,	TRUE  } //# is used for Hash tags. 
   //,{	(MYWCHAR)'$',			0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'%',			0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'^',			0, 0,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'(',			0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)')',			0, 0,	TRUE,	FALSE }
   //,{	(MYWCHAR)'{',			0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)'}',			0, 0,	TRUE,	FALSE }
   //,{	(MYWCHAR)'[',			0, 1,	FALSE,	TRUE  }
   //,{	(MYWCHAR)']',			0, 0,	TRUE,	FALSE }
   ,{	(MYWCHAR)'«',			0, 1,	FALSE,	TRUE  }  // u + 00AB // spanish italian, french quotations
   ,{	(MYWCHAR)'»',			1, 0,	TRUE,	FALSE }  // u + 00BB  // spanish italian, french quotations
   ,{	(MYWCHAR)'“',			0, 1,	FALSE,	TRUE  }  // u + 0x201e // german quotations
   ,{	(MYWCHAR)'”',			1, 0,	TRUE,	FALSE }  // u + 0x201d // german quotations
   ,{	(MYWCHAR)'„',			0, 1,	TRUE,	FALSE }  // u + 0x2033 // german quotations
   ,{	(MYWCHAR)'¡',			0, 1,	FALSE,	TRUE  }  // reversed exclamation spanish u+00A1
   ,{	(MYWCHAR)'·',			0, 0,	TRUE,	FALSE }  // middle dot, spanish u+00b7
   ,{	(MYWCHAR)'—',			0, 1,	TRUE,	TRUE  }  // em dash , spanish literal text u+2014
   ,{		   NUL,			0, 0,   FALSE,	FALSE }
};

//////////////////////////////////////////////////////////////////////////////
PuncChoice *findPunctuationChoiceAtEndOfWord(MYWCHAR endofwordchar)
{
	PuncChoice *pc = gPunctSymbols;
	static const int nAr  = { sizeof(gPunctSymbols) / sizeof(PuncChoice) };

	for (int i = 0; i < nAr; i++)
	{
		if (pc[i].puncChar == endofwordchar)
			return &pc[i];
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
PuncChoice *findPunctuationChoice(MYWCHAR letter)
{
	if (letter == SP)
		return NULL;
	PuncChoice *pc = gPunctSymbols;
	static const int nAr  = { sizeof(gPunctSymbols) / sizeof(PuncChoice) };
	for (int i = 0; i < nAr; i++)
	{
		if (pc[i].puncChar == letter)
		{
			return &pc[i];
		}
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////
bool isLetterOrNumber(MYWCHAR c)
{
	return ((c >= (MYWCHAR) 'A' && c <= (MYWCHAR) 'Z') ||
			(c >= (MYWCHAR) 'a' && c <= (MYWCHAR) 'z') ||
			 isLCSpecialCharacter(c) == TRUE ||
			 isUCSpecialCharacter(c) == TRUE ||
			(c >= (MYWCHAR) '0' && c <= (MYWCHAR) '9')) ? TRUE : FALSE;
}

///////////////////////////////////////////////////////////////////////
bool isEndOfWordPunctuation(MYWCHAR c)
{
	return (isEndOfSentencePunctuation(c) || c == HTAB || c==SP);
}

/*
int isalnum(int c) -- True if c is alphanumeric.
int isalpha(int c) -- True if c is a letter.
int isascii(int c) -- True if c is ASCII .
int iscntrl(int c) -- True if c is a control character.
int isdigit(int c) -- True if c is a decimal digit
int isgraph(int c) -- True if c is a graphical character.
int islower(int c) -- True if c is a lowercase letter
int isprint(int c) -- True if c is a printable character
int ispunct (int c) -- True if c is a punctuation character.
int isspace(int c) -- True if c is a space character.
int isupper(int c) -- True if c is an uppercase letter.
int isxdigit(int c) -- True if c is a hexadecimal digit

Character Conversion:

int toascii(int c) -- Convert c to ASCII .
tolower(int c) -- Convert c to lowercase.
int toupper(int c) -- Convert c to uppercase.
*/
////////////////////////////////////////////////////////////////////////////////
bool matchingPunctuations(MYWCHAR a, MYWCHAR b)
{
	char aA = (char)a;
	char bA = (char)b;
	if (aA == '(' && bA == ')')
		return TRUE;
	if (aA == '{' && bA == '}')
		return TRUE;
	if (aA == '[' && bA == ']')
		return TRUE;
	if (aA == '#' && bA == '#')
		return TRUE;
	if (aA == '<' && bA == '>')
		return TRUE;
	if (aA == '$' && bA == '$')
		return TRUE;
	if (aA == '&' && bA == '&')
		return TRUE;
	if (aA == '/' && bA == '/')
		return TRUE;
	if (aA == '\\' && bA == '\\')
		return TRUE;
	if (aA == '\"' && bA == '\"')
		return TRUE;
	if (aA == '\'' && bA == '\'')
		return TRUE;
	if (aA == '`' && bA == '`')
		return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// take care of prunning and inspecting begin/end and extra characters at the end of a word.
int declutterWord(MYWCHAR *text, int nValidNodes)
{
	int wordLen = mywcslen(text);
	if (wordLen ==0 || (isalnum(text[0]) && isalnum(text[wordLen-1])))
		return 0;

	if (ispunct(text[0]) && isalnum(text[wordLen-1]) && nValidNodes == wordLen) 
	{
		text = &text[1];
		return 1;
	}

	if (ispunct(text[wordLen-1])) 
	{
		text[wordLen-1] = NUL;
		return 1;
	}

	if (matchingPunctuations(text[0], text[wordLen-1]))
	{
		text[wordLen-1] = NUL;
		text = &text[0];
		return 0;
	}

	if (wordLen > nValidNodes)
	{
		// the word has more characters than the recognized word in one of the dictionaries.
		// now the task is figuring out whether these extra characters need to be removed
		// before adding the word into the learning pattern.
		int allowedTillIdx = nValidNodes;
		for ( ;allowedTillIdx < wordLen; allowedTillIdx++)
		{
			if (!isalnum(text[allowedTillIdx])) 
				break;
		}
		text[allowedTillIdx] = NUL;
		return wordLen-allowedTillIdx;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////
int containsSPs(MYWCHAR *text)
{
	int i = 0;
	int count = 0;
	while (text[i]) 
		count += (text[i++]==SP); 
	return count;
}


///////////////////////////////////////////////////////////////////////////
bool wordEndsOnNormalPunctuation(MYWCHAR endChar)
{
	PuncChoice *pc = findPunctuationChoiceAtEndOfWord(endChar);
	return pc && pc->canTerminate;
}

CPPExternClose