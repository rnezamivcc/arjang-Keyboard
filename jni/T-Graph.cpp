#include "StdAfx.h"
#include "T-Graph.h"
#include "dictionary.h"
#include "nGramHistory.h"
#include "autocorrect.h"

const char* NounPredictionArr1[] = {"is", "was", "will be" };
const char* NounPredictionArr2[] = {"are","were", "will be" };
extern CDictManager *wpDictManager;
//////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraphCompareCasPref(const void *a, const void *b)
{
	TGraphBlk *nodeA = (TGraphBlk *)a;
	TGraphBlk *nodeB = (TGraphBlk *)b;

	return nodeB->casPref - nodeA->casPref;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraphCompareNGram(const void *a, const void *b)
{
	TGraphBlk *nodeA = (TGraphBlk *)a;
	TGraphBlk *nodeB = (TGraphBlk *)b;

	return nodeB->NGram - nodeA->NGram;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraphCompareLearnedPref(const void *a, const void *b)
{
	TGraphBlk *nodeA = (TGraphBlk *)a;
	TGraphBlk *nodeB = (TGraphBlk *)b;

	return nodeB->LearnedPref - nodeA->LearnedPref;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
TGraph::TGraph(Dictionary *topdict)
{
	m_dict = topdict;
	reset();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::reset()
{
	m_curLetters  = NULL;
	m_basePref =0;
	m_TGraphHistory = NULL;
	m_PGraphArr.reset();
	m_TProps.ResetProps();	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::set(MYWCHAR* curHistory,  MYWCHAR* curWord, NGramHistory* hist)
{
	WLBreakIf(m_dict == NULL, "!!ERROR! TGraph::InitTGraph: dict is null!!\n");
	if(!curHistory)
		return;

	ShowInfo("MK Set original use:(%s)\n",toA(curHistory)); 

	m_TGraphHistory = hist;
	trimEndingSpaces(curHistory);
	NGramHistory	stHistory;
	
	if(GetSpaceCount(curWord) == 0 && mywcslen(curWord) > 1)
	{
		DeleteLastWordFromPhrase(curHistory);
		trimEndingSpaces(curHistory);
	}

	ShowInfo("MK Set actual use:(%s)\n\n",toA(curHistory)); 
	int n = mywcslen(curHistory);
	ChangeI(curHistory, n);
	int startPos = 0;
	for(int i =n-1; i >=0; i--)
	{
		if(isPunctuation(curHistory[i]))
		{
			startPos = i+1;
			break;
		}
	}
	
	int dictIdx = 0;
	for(int i=0; i < MAX_TGRAPH_HISTORY_NUM; i++)
	{
		if(i == 0)
		{
			SetNGramHistoryWord(curHistory, &stHistory, i, n, startPos);
		}
		else
		{
			if(stHistory.newSpaceIndex[i-1] > 0)
			{
				SetNGramHistoryWord(curHistory, &stHistory, i, n, stHistory.newSpaceIndex[i-1]);
			}
		}

		stHistory.historyNode[i] = wpDictManager->retrieveEndNodeForString(stHistory.word[i], &dictIdx, true);
		if(!stHistory.historyNode[i] && !isEmptyStr(stHistory.word[i]))
		{
			dictIdx = 0;
			stHistory.historyNode[i] = wpDictManager->QuadRetrieveCompactNode(stHistory.word[i], true, &dictIdx);
		}
	
	}

	//For words that not existed in dictionary
	for(int i =MAX_TGRAPH_HISTORY_NUM-1; i >=0; i--)
	{
		if(!isEmptyStr(stHistory.word[i]))
		{
			if(isEmptyStr(m_TProps.N2StartWord))	
				mywcscpy(m_TProps.N2StartWord, stHistory.word[i]);	
			else if(isEmptyStr(m_TProps.N3StartWord))
				mywcscpy(m_TProps.N3StartWord, stHistory.word[i]);
			else if(isEmptyStr(m_TProps.N4StartWord))
				mywcscpy(m_TProps.N4StartWord, stHistory.word[i]);		
		}
	}

	//After auto corrected, histoy should be arranged for nodes. 
	//Ex)"Got to hgld hold"-->"Got to hold".
	for( int i=0; i < MAX_TGRAPH_HISTORY_NUM; i++)
	{
		if(stHistory.historyNode[i] == NULL)
		{
			if(i+1 < MAX_TGRAPH_HISTORY_NUM && stHistory.historyNode[i+1])
			{
				stHistory.historyNode[i] = stHistory.historyNode[i+1];
				stHistory.historyNode[i+1] = NULL;

				mywcscpy(stHistory.word[i], stHistory.word[i+1]);
				memset(stHistory.word[i+1], 0, sizeof(stHistory.word[i+1]));
			}
		}
	}

	int nIndex = 0;
	for(int i =MAX_TGRAPH_HISTORY_NUM-1; i >=0; i--)
	{
		if(stHistory.historyNode[i])
		{
			nIndex = i;
			m_TProps.p2GramStartNode = stHistory.historyNode[i];
			mywcscpy(m_TProps.N2StartWord, stHistory.word[i]);
			break;
		}
	}


	for( int i=0; i < MAX_TGRAPH_HISTORY_NUM; i++)
	{
		if(stHistory.historyNode[i] == NULL && !isEmptyStr(stHistory.word[i]))
			memset(stHistory.word[i], 0, sizeof(stHistory.word[i]));

	}

	ShowInfo("MK Set history1:(%s) (%s) (%s) (%s)\n", toA(stHistory.word[0]), toA(stHistory.word[1]), toA(stHistory.word[2]), toA(stHistory.word[3]));
	m_TProps.p3GramStartNode = nIndex > 0 ? stHistory.historyNode[nIndex-1] : NULL;
	m_TProps.p4GramStartNode = nIndex > 1 ? stHistory.historyNode[nIndex-2] : NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::Process( MYWCHAR* curLetters, MultiLetterAdvanceAr *arr)
{
#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif

	ShowInfo("MK TGraph::Process:%s\n",toA(curLetters));
	WLBreakIf(m_dict == NULL, "!!ERROR!! dict for TGrpah is NULL!!!\n");
	if(curLetters)
		m_TProps.ResetTGraphBlk();
	
	if(arr)
		m_PGraphArr = *arr;

	m_basePref = wpDictManager->calcCascadingBasePreference(1, eOrdinary, 0);
	m_curLetters = curLetters;

	if(!isEmptyStr(m_curLetters))
	{
		int len = mywcslen(m_curLetters);
		if(m_curLetters[len-1] == APOSTROPHE)
		{
			//ShowInfo("MK TGraph::Process NLP APOSTROPHE\n");
			ProcessAPOSTROPHE();
			return;
		}
	}

	BOOL b2 = isEmptyStr(m_TProps.N2StartWord);
	BOOL b3 = isEmptyStr(m_TProps.N3StartWord);
	BOOL b4 = isEmptyStr(m_TProps.N4StartWord);

	bool bProcessTGraph = false;

	//////////////////////////////////2-gram////////////////////////////////////
	if(m_TProps.p2GramStartNode && !b2 && !m_TProps.p3GramStartNode && !m_TProps.p4GramStartNode)
	{
		Process2Gram();
		bProcessTGraph = true;
	}
	//////////////////////////////////3-gram////////////////////////////////////
	if(m_TProps.p2GramStartNode && !b2 && m_TProps.p3GramStartNode && !b3 && !m_TProps.p4GramStartNode && b4)
	{
		Process3Gram();
		bProcessTGraph = true;
	}
	/////////////////////////////////4-gram/////////////////////////////////////
	if(m_TProps.p2GramStartNode && !b2 && m_TProps.p3GramStartNode && !b3 && m_TProps.p4GramStartNode && !b4)
	{
		Process4Gram();
		bProcessTGraph = true;
	}
	///////////////////////////////////////////////////////////////////////////

	if(!bProcessTGraph && m_TProps.p4GramStartNode == NULL && m_TProps.p3GramStartNode && m_TProps.p2GramStartNode)
		Process3Gram();
	
	if(!bProcessTGraph && m_TProps.p4GramStartNode == NULL && m_TProps.p3GramStartNode == NULL && m_TProps.p2GramStartNode)	
		Process2Gram();


	Update();

#ifdef TIMETEST
	end = clock();
	double diff = Diffclock(end, start);
	ShowInfo("Time Final TGraph:%f\n", diff);
#endif
}
/*
Save prediction words for T-Graph and return -1 if P-Graph has the same word
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::SetSavedWords(MYWCHAR word[MAX_WORD_LEN], int index)
{
	WLBreakIf(index >= MAX_TGRAPH_SIZE, "!!ERROR! savedWords index is out of range!\n");
	bool bExist = false;
	int indexForDupliacte = -1;

	for(int i=0; i < MAX_TGRAPH_SIZE; i++)
	{
		if(m_TProps.stTGraphBlk[i].pNode == NULL)
			break;

		MYWCHAR* curWord = m_TProps.stTGraphBlk[i].GetTGraphWord();

		if(!isEmptyStr(curWord) &&  mywcscmp(word, curWord) ==0)
		{
			bExist =true;
			indexForDupliacte = i;
			break;
		}
	}

	if(!bExist && !m_curLetters)
	{
		indexForDupliacte = MAX_TGRAPH_SIZE;
		m_TProps.stTGraphBlk[index].SetTGraphNode(word);
	}
	else if(!bExist && m_curLetters && CheckLetter(word))
	{
		indexForDupliacte = MAX_TGRAPH_SIZE;
		m_TProps.stTGraphBlk[index].SetTGraphNode(word);
	}

	return indexForDupliacte;	
}
/*
If cur letter is null, it will find next words without checking letter. 
Ex)Type "to", then space, get 254 next words withour letter, but if letter is "c", find next words starting with "c".
*/
/////////////////////////////////////////////////////////////////////////////////////////////////
bool TGraph::CheckLetter(MYWCHAR *word)
{
	if(!m_curLetters || isEmptyStr(m_curLetters))
		return true;

	//you should not have same prediction as what you have been typing so far.  
	if(mywcscmp(m_curLetters, word) == 0)
		return false;

	for(int i=0; i < MAX_WORD_LEN && m_curLetters[i]; i++)
	{
		if(m_curLetters[i] != word[i])
			return false;	
	}

	return true;
}
/*
Any next words for T graph get preq*5 and if the word exists in P graph, get preq*50.
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::Update()
{
	int i=0,j=0;
	bool bNeedUpdate = false;
	SearchResultEntry* re= gRankedSearchResults;
	m_TProps.totalCount =0;
	for (i = 0; i < MAX_TGRAPH_SIZE; i++)
	{	
		MYWCHAR *TWord = m_TProps.stTGraphBlk[i].GetTGraphWord();
		if(isEmptyStr(TWord))
			break;

		if(CheckLetter(TWord))
		{	
			m_TProps.stTGraphBlk[i].casPref = m_TProps.stTGraphBlk[i].casPref*TGRAPH_ADD_PREFERENCE;

			//Updata P-Graph
			//for (j = 20; j < MAX_TGRAPH_SIZE; j++)
			for (j = 20; j < 30; j++)
			{
				SearchResultEntry *mysrep = &re[j];
				if(mysrep)
				{
					MYWCHAR *PWord = mysrep->text;
					if(PWord && TWord)
					{
						trimEndingSpaces(PWord);

						if(!isEmptyStr(PWord) && (mywcscmp(TWord, PWord) == 0))
						{
							mysrep->cascadingPref = m_TProps.stTGraphBlk[i].casPref;
							m_TProps.stTGraphBlk[i].casPref = m_TProps.stTGraphBlk[i].casPref*TGRAPH_ADD_PREFERENCE;
							//ShowInfo("MK Common Word[%d]: %s====%d\n", j, toA(PWord), mysrep->cascadingPref );		
						}
					}
				}
			}

			bNeedUpdate = true;
		}

		m_TProps.totalCount++;
	}

	if(bNeedUpdate)
		MergeIntoPGraph();

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::MergeIntoPGraph()
{
#ifdef TIMETEST
	clock_t start, end;
	start = clock();
#endif

	qsort(m_TProps.stTGraphBlk, m_TProps.totalCount, sizeof(TGraphBlk), TGraphCompareCasPref);

	if(m_TProps.IsNGramProcessed())
	{
		qsort(m_TProps.stTGraphBlk, m_TProps.totalCount, sizeof(TGraphBlk), TGraphCompareNGram);
		if(m_TProps.IsOnly2Grams())
			qsort(m_TProps.stTGraphBlk, m_TProps.totalCount, sizeof(TGraphBlk), TGraphCompareCasPref);
		else
			m_TProps.SortRest();	
	}

	if(m_TProps.IsLearnedNGramProcessed())
		qsort(m_TProps.stTGraphBlk, m_TProps.totalCount, sizeof(TGraphBlk), TGraphCompareLearnedPref);

	MultiLetterAdvanceAr tempArr;
	tempArr = m_PGraphArr;
	m_PGraphArr.reset();

	int nIndex =0;
	for(int i=0; i<NMWORDPREDICTIONS; i++)
	{
		if(isEmptyStr(tempArr.nextWords[i]))
			break;

		trimEndingSpaces(tempArr.nextWords[i]);

		MYWCHAR *curWord = m_TProps.stTGraphBlk[i].GetTGraphWord();
		if(isEmptyStr(curWord))
			break;

		bool bExist= false;
		for(int j=0; j <NMWORDPREDICTIONS;j++)
		{	
			MYWCHAR *p2 = m_PGraphArr.nextWords[j];
			if(isEmptyStr(p2))
				break;

			if (mywcscmp(curWord, p2) == 0)
			{
				bExist = true;
				break;
			}
		}

		if(CheckLetter(curWord) && !bExist)
		{
			m_PGraphArr.prefs[nIndex] = m_TProps.stTGraphBlk[i].casPref; 
			mywcscpy(m_PGraphArr.nextWords[nIndex], curWord);
			m_PGraphArr.nShouldUpperCase[nIndex] = m_TProps.stTGraphBlk[i].nUpperCase;
			nIndex++;			
		}
	}

	for(int i= 0; i< NMWORDPREDICTIONS; i++)
	{
		if(isEmptyStr(tempArr.nextWords[i]))
			break;

		if(tempArr.prefs[i]!=0xffff)
		{
			MYWCHAR *p1= tempArr.nextWords[i];
			bool bExist= false;
			for(int j=0; j <NMWORDPREDICTIONS;j++)
			{		
				MYWCHAR *p2 = m_PGraphArr.nextWords[j];
				if (p1 && p2 && mywcscmp(p1, p2) == 0)
				{
					bExist = true;
					break;
				}
			}

			if(!bExist)
			{
				bool bOK=false;
				for(nIndex=0; nIndex <NMWORDPREDICTIONS;nIndex++)
				{
					MYWCHAR *p3 = m_PGraphArr.nextWords[nIndex];
					if(isEmptyStr(p3))
					{
						bOK = true;
						break;
					}
				}
				if(bOK)
				{
					WLBreakIf(nIndex >= NMWORDPREDICTIONS,"!!UpdateTGraph nIndex ERROR!!!!\n");
					m_PGraphArr.prefs[nIndex] = tempArr.prefs[i];
					mywcscpy(m_PGraphArr.nextWords[nIndex], tempArr.nextWords[i]);
					m_PGraphArr.nShouldUpperCase[nIndex] = tempArr.nShouldUpperCase[i];
				}
			}
		}
	}

	int resultCount =0;
	for(int i= 0; i< NMWORDPREDICTIONS; i++)
	{
		MYWCHAR *p = m_PGraphArr.nextWords[i];

		trimEndingSpaces(p);

		if(isEmptyStr(p))
			break;

		wpDictManager->ConvertWordForChunk(p);

		//ShowInfo("MK T-Graph Result:(%s)\n",toA(m_PGraphArr.nextWords[i]));
		resultCount++;
	}

	if(resultCount == 0)
	{
		for(int i= 0; i< NMWORDPREDICTIONS; i++)
		{
			MYWCHAR* curWord = m_TProps.stTGraphBlk[i].GetTGraphWord();
			if(m_TProps.stTGraphBlk[i].pNode == NULL || isEmptyStr(curWord))
				break;
			//ShowInfo("MK curWord:%s,     pref:%hu,      NGram:%d\n", toA(curWord), m_TProps.stTGraphBlk[i].casPref, m_TProps.stTGraphBlk[i].NGram);
			mywcscpy(m_PGraphArr.nextWords[i], curWord);
			m_PGraphArr.prefs[i] = m_TProps.stTGraphBlk[i].casPref;
			m_PGraphArr.nShouldUpperCase[i] = m_TProps.stTGraphBlk[i].nUpperCase;
			wpDictManager->ConvertWordForChunk(m_PGraphArr.nextWords[i]);
		}
	}


	for(int i= 0; i< NMWORDPREDICTIONS; i++)
	{
		MYWCHAR* curWord = m_TProps.stTGraphBlk[i].GetTGraphWord();
		if(m_TProps.stTGraphBlk[i].pNode == NULL || isEmptyStr(curWord))
			break;

		if(mywcscmp(curWord, toW(NO_DICT_INDICATOR)) ==0)
			mywcscpy(m_PGraphArr.nextWords[i], m_TProps.SavedLearnedWord[i]);
	
	}


	wpDictManager->SetProcessedArr(m_PGraphArr);

	//for(int i=0; i< NMWORDPREDICTIONS; i++)
	//{
	//	ShowInfoIf(!isEmptyStr(m_PGraphArr.nextWords[i]), "MK Test2:%s\n", toA(m_PGraphArr.nextWords[i]));

	//}
#ifdef TIMETEST
	end = clock();
	double diff = Diffclock(end, start);
	ShowInfo("Time MergeIntoPGraph:%f\n", diff);
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::ProcessNouns(MYWCHAR* curWord)
{
	if(!m_dict || m_dict->GetDictLanguage() != eLang_ENGLISH || !m_TGraphHistory)
		return;

	//ShowInfo("MK ProcessNouns word:(%s)\n", toA(curWord));
	if(!isEmptyStr(curWord))//Hasn't typed space or advanceWord..still typing letters now.
	{
		MYWCHAR* temp = GetLastWordFromPhrase(m_TGraphHistory->NewCurrentHistory);
		for(int i=0; temp[i] && i < MAX_WORD_LEN; i++)
		{
			if(isUpperCase(temp[i]) && curWord[i])
				curWord[i] = uprCharacter(curWord[i]);

		}
	
		int dictIdx =0;
		CompactNode* node =  wpDictManager->retrieveEndNodeForString(curWord, &dictIdx, true);
		if(!node)
			node = wpDictManager->QuadRetrieveCompactNode(curWord, true, &dictIdx);

		if(IsNodeProperNoun(node))
		{
			curWord = GetApostropheWord(curWord);
			if(curWord)
			{
				for(int i=0; i < NMWORDPREDICTIONS;i++)
				{
					if(isEmptyStr(m_PGraphArr.nextWords[i]))
					{
						mywcscpy(m_PGraphArr.nextWords[i], curWord);
						wpDictManager->ConvertWordForChunk(m_PGraphArr.nextWords[i]);
						m_PGraphArr.prefs[i] = m_basePref+TGRAPH_ADD_PREFERENCE;
						wpDictManager->SetProcessedArr(m_PGraphArr);
						break;
					}
				}


			}
		}

	}
	else
	{	
		CompactNode *node = m_TProps.p2GramStartNode;
		if(!node || node->POStag == ePOS_NOTAG)
			return;
		
		bool bGetNounPrediction = false;
		if(node->POStag == ePOS_NC || node->POStag == ePOS_NP || node->POStag == ePOS_NM || node->POStag == ePOS_AP)
		{
			SetNounPredictionArr(false);			
			bGetNounPrediction = true;
		}
		else if(node->POStag == ePOS_NPS || node->POStag == ePOS_NS)
		{
			SetNounPredictionArr(true);
			bGetNounPrediction = true;
		}

		if(!bGetNounPrediction)
			return;

	
		for(int i=0; i < NMWORDPREDICTIONS;i++)
		{
			if(m_PGraphArr.prefs[i] == 0xffff && !isEmptyStr(m_PGraphArr.nextWords[i]))
			{
				wpDictManager->ConvertWordForChunk(m_PGraphArr.nextWords[i]);
				m_PGraphArr.prefs[i] = m_basePref+TGRAPH_ADD_PREFERENCE+i;
			}
		}

		wpDictManager->SetProcessedArr(m_PGraphArr);	
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::SetNounPredictionArr(bool plural)
{
	for(int i=0; i<3; i++)
	{
		for(int k=0; k < NMWORDPREDICTIONS;k++)
		{
			if(isEmptyStr(m_PGraphArr.nextWords[k]))
			{
				if(!plural)
					mywcscpy(m_PGraphArr.nextWords[k], toW(NounPredictionArr1[i]));
				else
					mywcscpy(m_PGraphArr.nextWords[k], toW(NounPredictionArr2[i]));
				break;
			}
		}
	}

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::ProcessAPOSTROPHE()
{
	if(!m_dict || m_dict->GetDictLanguage() != eLang_ENGLISH)
		return;

	MYWCHAR* apoWord = GetLastWordFromPhrase(m_TGraphHistory->HistoryForPhrase);
	if(isEmptyStr(apoWord))
		return;

	ShowInfo("MK ProcessAPOSTROPHE word:(%s)\n", toA(apoWord));
	bool bAllow = false;
	if(isUpperCase(apoWord[0]))
	{
		bAllow = true;
	}
	else
	{	
		int dictIdx =0;
		CompactNode* newNode = wpDictManager->retrieveEndNodeForString(apoWord, &dictIdx, true);
		if(IsNodeProperNoun(newNode))
		{
			bAllow = true;
		}
		else
		{
			if(isUpperCase(apoWord[0]))
				apoWord[0] = lwrCharacter(apoWord[0]);
			else
				apoWord[0] = uprCharacter(apoWord[0]);

			newNode = wpDictManager->retrieveEndNodeForString(apoWord, &dictIdx, true);
			if(IsNodeProperNoun(newNode))
			{
				bAllow = true;
			}
		}
	}

	if(!bAllow)
		return;

	m_PGraphArr.reset();
	m_TProps.ResetTGraphBlk();

	apoWord = GetApostropheWord(apoWord);
	if(apoWord)
	{
		apoWord[0] = uprCharacter(apoWord[0]);
		mywcscpy(m_PGraphArr.nextWords[0], apoWord);
		wpDictManager->ConvertWordForChunk(m_PGraphArr.nextWords[0]);
		m_PGraphArr.prefs[0] = m_basePref+TGRAPH_ADD_PREFERENCE;
		wpDictManager->SetProcessedArr(m_PGraphArr);

		ShowInfo("MK ProcessAPOSTROPHE completed\n");
	}
}

bool TGraph::IsNodeProperNoun(CompactNode* node)
{

	if(node && (node->POStag == ePOS_NP || m_dict->getCompactStore()->getSecondPOSTag(node) == ePOS_NP))
		return true;

	return false;
}

MYWCHAR* TGraph::GetApostropheWord(MYWCHAR* word)
{
	if(isEmptyStr(word))
		return NULL;

	int len = mywcslen(word);
	if(word[len-1] == 's' || word[len-1] == 'S')
		mywcscat(word, toW("'"));
	else
		mywcscat(word, toW("'s"));

	return word;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void TGraph::SortTGraph(UINT value[MAX_TGRAPH_SIZE])
//{
//	bool bFlag = true;   
//	int pref = 0; 
//	int NGram = 0;
//	int LearnedPref = 0;
//	MYWCHAR words[MAX_TGRAPH_SIZE][MAX_WORD_LEN];
//	for(int i = 1; (i <= MAX_TGRAPH_SIZE) && bFlag; i++)
//	{
//		bFlag = false;
//		for (int j=0; j < (MAX_TGRAPH_SIZE -1); j++)
//		{
//			if(isEmptyStr(m_TProps.savedWords[j]))
//				break;
//
//			if (value[j+1] > value[j])     
//			{ 
//				pref = m_TProps.casPref[j];
//				NGram = m_TProps.NGram[j];  
//				LearnedPref = m_TProps.LearnedPref[j];
//	
//				mywcscpy(words[j], m_TProps.savedWords[j]);
//				m_TProps.casPref[j] = m_TProps.casPref[j+1];
//				m_TProps.casPref[j+1] = pref;
//				m_TProps.NGram[j] = m_TProps.NGram[j+1];
//				m_TProps.NGram[j+1] = NGram;
//				m_TProps.LearnedPref[j] = m_TProps.LearnedPref[j+1];
//				m_TProps.LearnedPref[j+1] = LearnedPref;
//
//				mywcscpy(m_TProps.savedWords[j], m_TProps.savedWords[j+1]);
//				mywcscpy(m_TProps.savedWords[j+1], words[j]);
//							
//				bFlag = true;             
//			}
//		}
//	}
//}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function switches i to I in cases when it is a sole word! It assumes len == mywcslen(curHistory) !
void TGraph::ChangeI(MYWCHAR* curHistory, int len)
{
	//Minkyu:2014.02.05
	if(m_dict && m_dict->GetDictLanguage() == eLang_ENGLISH)
	{
		//WLBreakIf(len>1 && (curHistory[0]==SP || curHistory[len-1]==SP), "!!WARNING!!ChangeI:  history should not start or end with SP!\n");
		if(len==1 && curHistory[0] == 'i')
			curHistory[0] = 'I';
		else if(len==2 && isNotText(curHistory[0]) && curHistory[1] == 'i')
			curHistory[1] = 'I';

		for(int i=2; i <len; i++)
		{
			if(curHistory[i] == 'i' && isNotText(curHistory[i-1]) && isNotText(curHistory[i+1]))
			{
				if(i+1 < len && (curHistory[i+1] == SP || curHistory[i+1] == NUL))
					curHistory[i] = 'I';
				if(i == len-1 && (curHistory[i+1] == SP || curHistory[i+1] == NUL))
					curHistory[i] = 'I';
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::SetNGramHistoryWord(MYWCHAR* pHistory, NGramHistory* stHistory, int arrIndex, int length, int nSpaceIndex)
{
	int newSpaceIndex =0;
	int index =0;
	for(int i=nSpaceIndex; i <length; i++)
	{
		if(pHistory[i] == SP)
			nSpaceIndex++;
		else
			break;
	}

	for(int i=nSpaceIndex; i <length; i++)
	{
		if(pHistory[i] == SP)
		{
			newSpaceIndex = i+1;
			break;
		}

		stHistory->word[arrIndex][index] = pHistory[i];
		index++;
	}

	if(stHistory->word[arrIndex][0] == 'i' && stHistory->word[arrIndex][1] == APOSTROPHE)
		//stHistory->word[arrIndex][0] = uprCharacter(stHistory->word[arrIndex][0]);
		ReplaceiToI(stHistory->word[arrIndex]);
	

	stHistory->newSpaceIndex[arrIndex] = newSpaceIndex;
	return newSpaceIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::Process2Gram()
{
	if(!m_TProps.p2GramStartNode || !m_dict)
		return MAX_TGRAPH_SIZE;

	MYWCHAR* cmpWord = NULL;
	if(!isEmptyStr(m_curLetters))
		cmpWord = m_curLetters;

	int nextIndex= 0;
	bool b2Gram = false;
	int MaxNGrams = wpDictManager->setNGramFrom2Grams(m_TProps.p2GramStartNode, NULL, cmpWord);
	WLBreakIf(MaxNGrams > MAX_FOUND_PHRASES, "!!ERROR! MaxNGrams > 255 for %s!!\n", toA(m_TProps.N2StartWord));
	for(int k=0; k < MaxNGrams; k++)
	{
		WLBreakIf(PhraseEngineR::gPhrases[k].isSet() == false, "!!ERROR! TGraph: g3Grams[%i] is not set!!\n", k);
		if(nextIndex >= MAX_TGRAPH_SIZE)
			break;

		int len=0;
		MYWCHAR **words = PhraseEngineR::gPhrases[k].getStr(len);
		if(SetSavedWords(words[1], nextIndex) == MAX_TGRAPH_SIZE)
		{
			USHORT uPref = PhraseEngineR::gPhrases[k].pref;
			m_TProps.stTGraphBlk[nextIndex].casPref = m_basePref+uPref;
			m_TProps.stTGraphBlk[nextIndex].NGram= N2Gram;
			b2Gram  = true;
			nextIndex++;
		}
		
	}

	MYWCHAR secondWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN];
	memset(secondWord, 0, sizeof(secondWord));

	USHORT learnedPref[MAX_TGRAPH_SIZE];
	memset(learnedPref, 0, sizeof(learnedPref));

	bool bIgnorePref = false;
	if(MaxNGrams == 0)
		bIgnorePref = true;

	int n2Count = wpDictManager->GetNGramLearning()->Find2LearningWord(secondWord, learnedPref, m_dict, 
		m_TProps.p2GramStartNode, cmpWord, m_TProps.N2StartWord, bIgnorePref);


	int NoDict2Count = wpDictManager->GetNGramLearning()->Find2LearningNoDicWord(m_TProps.N2StartWord, secondWord, learnedPref, cmpWord, bIgnorePref);
	if(n2Count + NoDict2Count == 0)
	{
		if(!b2Gram)
			return MAX_TGRAPH_SIZE;
		else
			return nextIndex;

	}

	int nCount =nextIndex;
	int nAvailable = MAX_TGRAPH_SIZE-nCount;
	if(n2Count > nAvailable)
		nCount = nCount - (n2Count-nAvailable);

	if(nextIndex == MAX_TGRAPH_SIZE)	
		nCount =0; 


	int ind = SetLearningTGraph(secondWord, learnedPref, N2Gram, nCount, 0);

	if(!b2Gram)
		return MAX_TGRAPH_SIZE;
	else
		return ind;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::Process3Gram()
{
	if(!m_TProps.p2GramStartNode || !m_TProps.p3GramStartNode || !m_dict)
		return;
	
	//ShowInfo("MK Process3Gram p3GramStart(%s) && p2GramStart(%s) count:%d\n",toA(m_TProps.N3StartWord),toA(m_TProps.N2StartWord));

	MYWCHAR thirdWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN];
	memset(thirdWord, 0, sizeof(thirdWord));

	USHORT learnedPref[MAX_TGRAPH_SIZE];
	memset(learnedPref, 0, sizeof(learnedPref));

	USHORT uPref = 0;
	int n3Count = Set3GramWord(thirdWord, learnedPref, &uPref);
	if(n3Count==0)
	{
		Process2Gram();
		return;
	}

	int nextIndex = Process2Gram();

	int nCount =nextIndex;
	int nAvailable = MAX_TGRAPH_SIZE-nCount;
	if(n3Count > nAvailable)
		nCount = nCount - (n3Count-nAvailable);

	if(nextIndex == MAX_TGRAPH_SIZE)
		nCount =0;

	SetLearningTGraph(thirdWord, learnedPref, N3Gram, nCount, uPref);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::SetLearningTGraph(MYWCHAR learnedWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], 
							   int nGram, int start,  USHORT uPref)
{
	if(nGram == N2Gram)
		uPref = TGRAPH_ADD_PREFERENCE;


	memset(m_TProps.SavedLearnedWord, 0, sizeof(m_TProps.SavedLearnedWord));
	int ind =0;
	for(int k=start; k < MAX_TGRAPH_SIZE;k++)
	{
		if(isEmptyStr(learnedWord[ind]))
			break;
		
		BYTE nUpperValue = 0;
		if(isUpperCase(learnedWord[ind][0]) && mywcscmp(learnedWord[ind], toW("I")) != 0)
		{
			MakeLowerCase(learnedWord[ind]);
			nUpperValue = 1;
		}

		int nResult = SetSavedWords(learnedWord[ind], k);
		if(nResult == MAX_TGRAPH_SIZE)
		{
			bool bSet = false;
			int dictIdx =0;
			if(m_TProps.stTGraphBlk[k].pNode)
			{
				CompactNode* node = wpDictManager->retrieveEndNodeForString(learnedWord[ind], &dictIdx, true);
				if(node != m_TProps.stTGraphBlk[k].pNode)
				{
					m_TProps.stTGraphBlk[k].pNode = wpDictManager->retrieveEndNodeForString(toW(NO_DICT_INDICATOR), &dictIdx, true);
					mywcscpy(m_TProps.SavedLearnedWord[ind], learnedWord[ind]);
				}
				bSet = true;
			}
			else
			{
				
				m_TProps.stTGraphBlk[k].pNode = wpDictManager->retrieveEndNodeForString(toW(NO_DICT_INDICATOR), &dictIdx, true);
				if(m_TProps.stTGraphBlk[k].pNode)
				{				
					mywcscpy(m_TProps.SavedLearnedWord[ind], learnedWord[ind]);
					bSet = true;
				}
			}

			if(bSet)
			{
				m_TProps.stTGraphBlk[k].casPref= m_basePref+uPref;
				m_TProps.stTGraphBlk[k].NGram = nGram;	
				m_TProps.stTGraphBlk[k].LearnedPref = learnedPref[ind];
				m_TProps.stTGraphBlk[k].nUpperCase= nUpperValue;
			}

		}
		else
		{
			if(m_TProps.stTGraphBlk[nResult].pNode)
			{
				m_TProps.stTGraphBlk[nResult].NGram = nGram;
				m_TProps.stTGraphBlk[nResult].LearnedPref = learnedPref[ind];
				m_TProps.stTGraphBlk[nResult].nUpperCase= nUpperValue;
			}

		}

		ind++;
	}

	return ind;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::Set3GramWord(MYWCHAR thirdWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], USHORT *uPref)
{
	int count =0;
	if(wpDictManager->GetNGramLearning())
		count = wpDictManager->GetNGramLearning()->Find3LearningWord(thirdWord, learnedPref, m_dict, m_TProps.p3GramStartNode, m_TProps.p2GramStartNode);

	if(count >= MAX_TGRAPH_SIZE)
		return count;

	MYWCHAR* cmpWord = NULL;
	if(!isEmptyStr(m_curLetters))
		cmpWord = m_curLetters;

	int MaxNGrams = wpDictManager->setNGramFrom2Grams(m_TProps.p3GramStartNode, m_TProps.p2GramStartNode, cmpWord);
	WLBreakIf(MaxNGrams > MAX_FOUND_PHRASES, "!!ERROR! MaxNGrams > 255 for %s and %s!!\n", toA(m_TProps.N3StartWord), toA(m_TProps.N4StartWord));

	for(int k=0; k < MaxNGrams;k++)
	{
		WLBreakIf(PhraseEngineR::gPhrases[k].isSet() == false, "!!ERROR! TGraph: Set3GramWord:gPhrases[%i] is not set!!\n", k);
		if(count >= MAX_TGRAPH_SIZE)
			break;

		int phraseLen = PhraseEngineR::gPhrases[k].nGrams;
		if(phraseLen >= N3Gram)
		{
			MYWCHAR **words = PhraseEngineR::gPhrases[k].getStr(phraseLen);
			for(int index=0; index <  MAX_TGRAPH_SIZE; index++)
			{
				if(isEmptyStr(thirdWord[index]))
				{
					mywcscpy(thirdWord[index], words[2]);
					*uPref = PhraseEngineR::gPhrases[k].pref;
					count++;
					break;
				}
			}
		}
	}

	return count;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::Process4Gram()
{
	if(!m_TProps.p2GramStartNode || !m_TProps.p3GramStartNode || !m_TProps.p4GramStartNode || !m_dict)
		return;
	
	//ShowInfo("MK Process4Gram p4GramStart(%s) && p3GramStart(%s) && p2GramStart(%s)\n",toA(m_TProps.N4StartWord),toA(m_TProps.N3StartWord),toA(m_TProps.N2StartWord));

	MYWCHAR fourthWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN];
	memset(fourthWord, 0, sizeof(fourthWord));

	USHORT learnedPref[MAX_TGRAPH_SIZE];
	memset(learnedPref,0,sizeof(learnedPref));

	USHORT uPref = 0;
	int n4Count=Set4GramWord(fourthWord, learnedPref, &uPref);
	if( n4Count ==0)
	{
		Process3Gram();
		return;
	}

	int nextIndex = Process2Gram();

	int nCount =nextIndex;
	int nAvailable = MAX_TGRAPH_SIZE-nCount;
	if(n4Count > nAvailable)
		nCount = nCount - (n4Count-nAvailable);
	
	if(nextIndex == MAX_TGRAPH_SIZE)
		nCount =0;
	
	SetLearningTGraph(fourthWord, learnedPref, N4Gram, nCount, uPref);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::Set4GramWord(MYWCHAR fourthWord[MAX_TGRAPH_SIZE][MAX_WORD_LEN], USHORT learnedPref[MAX_TGRAPH_SIZE], USHORT* uPref)
{

	int count =0;
	if(wpDictManager->GetNGramLearning())
	{
		count = wpDictManager->GetNGramLearning()->Find4LearningWord(fourthWord, learnedPref, m_dict, 
			m_TProps.p4GramStartNode, m_TProps.p3GramStartNode, m_TProps.p2GramStartNode);
	}

	if(count >= MAX_TGRAPH_SIZE)
		return count;

	MYWCHAR* cmpWord = NULL;
	if(!isEmptyStr(m_curLetters))
		cmpWord = m_curLetters;

	int MaxNGrams = wpDictManager->setNGramFrom2Grams(m_TProps.p4GramStartNode, m_TProps.p3GramStartNode, cmpWord);
	WLBreakIf(MaxNGrams > MAX_FOUND_PHRASES, "!!ERROR! MaxNGrams > 255 for %s and %s!!\n", toA(m_TProps.N4StartWord), toA(m_TProps.N3StartWord));

	for(int k=0; k < MaxNGrams; k++)
	{
		WLBreakIf(PhraseEngineR::gPhrases[k].isSet() == false, "!!ERROR! TGraph: g3Grams[%i] is not set!!\n", k);
		if(count >= MAX_TGRAPH_SIZE)
			break;

		int phraseLen = PhraseEngineR::gPhrases[k].nGrams;
		
		if(phraseLen == N4Gram)
		{
			MYWCHAR **words = PhraseEngineR::gPhrases[k].getStr(phraseLen);
			for(int index=0; index <  MAX_TGRAPH_SIZE; index++)
			{
				if(isEmptyStr(fourthWord[index]))
				{
					//"Going to be" -->Not 4 gram
					//"Going to the mall"-->4 gram
					//Need to check 2gram start word to make sure if this is really 4 gram. 
					MYWCHAR* lastWord = words[2];
					if(!isEmptyStr(lastWord) && mywcscmp(lastWord, m_TProps.N2StartWord) == 0)
					{
						mywcscpy(fourthWord[index], words[3]);
						*uPref = PhraseEngineR::gPhrases[k].pref;
						count++;			
					}

					break;

				}
			}
		}
	}

	return count;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraph::UpdateAutoCorrect(MYWCHAR* input, MultiLetterAdvanceAr* pArr)
{
	if(isEmptyStr(m_curLetters) && !input)
		return;

	if(input && pArr)
	{
		m_curLetters = input;
		m_PGraphArr = *pArr;
		
	}

	MakeLowerCase(m_curLetters);
	ShowInfo("MK UpdateAutoCorrect m_curLetters:[%s]\n", toA(m_curLetters));

	wpDictManager->getAutocorrect()->ClearBuffer();
	int len = mywcslen(m_curLetters);
	for(int i=0; i<len; i++)
		wpDictManager->getAutocorrect()->addChar(m_curLetters[i]);

	CorrectionList * corrList = wpDictManager->getAutocorrect()->getPredictions(5);
	if(!corrList)
		return;

	BYTE totalCorrectionCount = corrList->fillCount;
	if(totalCorrectionCount == 0)
		return;

	
	MultiLetterAdvanceAr arr;
	arr.reset();
	int curInd = 0;
	int ret = GetDuplicated(&m_PGraphArr, corrList->corrections[0].word);
	if(ret == NMWORDPREDICTIONS)
	{
		mywcscpy(arr.nextWords[0], corrList->corrections[0].word); //Only add top 1 from auto correct.
		arr.prefs[0] = MAX_PREFERENCE;
		arr.bCorrection[0] = true;
		arr.nCorrections++;	
		curInd = 1;
		ShowInfo("MK UpdateAutoCorrect top1:%s[0], fillCount:%d\n", toA(corrList->corrections[0].word), totalCorrectionCount);//use top one
	}
	else
	{
		arr.bCorrection[ret] = true;
		ShowInfo("MK UpdateAutoCorrect same P and C:%s\n", toA(corrList->corrections[0].word));
	}

	for(int i=0; i < NMWORDPREDICTIONS;i++) //add predictions for rest of spots 
	{
		if(curInd >= NMWORDPREDICTIONS || isEmptyStr(m_PGraphArr.nextWords[i]))
			break;
		if(GetDuplicated(&arr, m_PGraphArr.nextWords[i]) == NMWORDPREDICTIONS)
		{
			mywcscpy(arr.nextWords[curInd], m_PGraphArr.nextWords[i]);
			ShowInfo("MK UpdateAutoCorrect Predictions:%s[%d]\n", toA(arr.nextWords[curInd]), curInd);
			arr.prefs[curInd] = m_PGraphArr.prefs[i];
			curInd++;
		}

		arr.nShouldUpperCase[i] = m_PGraphArr.nShouldUpperCase[i];
	}
			
	if(curInd < NMWORDPREDICTIONS && totalCorrectionCount > 1) //There are more auto corrections to fill this array if empty spot is available.
	{
		int startInd = 0;
		while(startInd < totalCorrectionCount)
		{
			MYWCHAR* word = corrList->corrections[startInd].word;
			if(word && GetDuplicated(&arr, word) == NMWORDPREDICTIONS && curInd < NMWORDPREDICTIONS)
			{
				mywcscpy(arr.nextWords[curInd], word);
				ShowInfo("MK UpdateAutoCorrect Corrections:%s[%d]\n", toA(arr.nextWords[curInd]), curInd);
				arr.prefs[curInd] = MIN_PREFERENCE;
				arr.nShouldUpperCase[curInd] = m_PGraphArr.nShouldUpperCase[0]; //Get from first word	
				arr.nCorrections++;
				arr.bCorrection[curInd] = true;
				curInd++;
			}
			startInd++;
		}
	}
	arr.nBackSpace = m_PGraphArr.nBackSpace;
	wpDictManager->SetProcessedArr(arr);
	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int TGraph::GetDuplicated(MultiLetterAdvanceAr* arr, MYWCHAR* word)
{
	for(int i=0; i<NMWORDPREDICTIONS; i++)
	{
		if(isEmptyStr(arr->nextWords[i]))
			break;
		if(mywcscmp(word, arr->nextWords[i]) == 0)
			return i;
	}

	return NMWORDPREDICTIONS;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraphProps::SortRest()
{

	TGraphBlk		temp[MAX_TGRAPH_SIZE];
	memset(temp, 0, sizeof(temp));

	int index =0;
	for(int i=0; i < totalCount; i++)
	{
		if(stTGraphBlk[i].pNode && stTGraphBlk[i].NGram > N2Gram)
		{
			temp[index++] = stTGraphBlk[i];
		}
	}
	if(index > 1)
		qsort(temp, index, sizeof(TGraphBlk), TGraphCompareCasPref);

	qsort(stTGraphBlk, totalCount, sizeof(TGraphBlk), TGraphCompareCasPref);

	for(int i=0; i < totalCount; i++)
	{
		if(stTGraphBlk[i].pNode && stTGraphBlk[i].NGram < N3Gram)
			temp[index++] = stTGraphBlk[i];
	}

	memset(stTGraphBlk, 0, sizeof(stTGraphBlk));
	for(int i=0; i < totalCount; i++)
	{
		stTGraphBlk[i] = temp[i];
	}

	//for(int i= 0; i< NMWORDPREDICTIONS; i++)
	//{
	//	MYWCHAR* curWord = stTGraphBlk[i].GetTGraphWord();
	//	if(stTGraphBlk[i].pNode == NULL || isEmptyStr(curWord))
	//		break;
	//	ShowInfo("MK SortRest:%s,     pref:%hu,      NGram:%d\n", toA(curWord), stTGraphBlk[i].casPref, stTGraphBlk[i].NGram);
	//}
		
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool TGraphProps::IsNGramProcessed()
{
	for(int i=0; i < MAX_TGRAPH_SIZE; i++)
	{
		if(stTGraphBlk[i].pNode == NULL)
			break;

		if(stTGraphBlk[i].NGram > 0)
			return true;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool TGraphProps::IsOnly2Grams()
{
	for(int i=0; i < MAX_TGRAPH_SIZE; i++)
	{
		if(stTGraphBlk[i].pNode == NULL)
			break;

		if(stTGraphBlk[i].NGram != N2Gram)
			return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool TGraphProps::IsLearnedNGramProcessed()
{
	for(int i=0; i < MAX_TGRAPH_SIZE; i++)
	{
		if(stTGraphBlk[i].pNode == NULL)
			break;

		if(stTGraphBlk[i].LearnedPref > 0)
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void TGraphProps::ResetTGraphBlk()
{
	for(int i=0; i < MAX_TGRAPH_SIZE; i++)
	{
		stTGraphBlk[i].pNode = NULL;
		stTGraphBlk[i].NGram =0;
		stTGraphBlk[i].casPref =0;
		stTGraphBlk[i].LearnedPref =0;
	}
}

MYWCHAR* TGraphBlk::GetTGraphWord()
{
	if(!pNode)
		return NULL;

	return wpDictManager->retrieveStringForEndNode(pNode);
}
void TGraphBlk::SetTGraphNode(MYWCHAR *word)
{
	int dictIdx =0;
	CompactNode* node = wpDictManager->retrieveEndNodeForString(word, &dictIdx, true);
	if(node)
		pNode = node;

}
