
#include "StdAfx.h"
#include "treatmentruleserializer.h"
//#include "compactstore.h"
#include "utility.h"

TreatmentRuleSerializer::TreatmentRuleSerializer() 
{
	m_treatmentRules = NULL;
	m_nRules = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void *TreatmentRuleSerializer::initialize(LangTreatmentRule *treatmentRules, int *nBytesP)
{
	m_treatmentRules = treatmentRules;

	int nStrChars = 0;
	for (int i = 0; m_treatmentRules[i].triggerChar; i++)
	{
	//	nStrChars +=  1; // for m_treatmentRules[i].triggerChar
		nStrChars += mywcslen(m_treatmentRules[i].triggerStr) + 1;
		nStrChars += mywcslen(m_treatmentRules[i].replacementStr) + 1;
		m_nAllocatedPtrs += 2; // three pointer in every LangTreatmentRule
		m_nRules++;
	}
	m_nRules++;


	m_nAllocSerializedBytes = m_nAllocatedPtrs*sizeof(GENPTR) + nStrChars*sizeof(MYWCHAR) + m_nRules*sizeof(LangTreatmentRule);
	m_nAllocSerializedBytes = (m_nAllocSerializedBytes + 0x3) & ~0x3;

	m_serializedMemory = (char *) calloc(1, m_nAllocSerializedBytes);
	m_handledPtrs = (KnownPtr *) calloc(1, m_nAllocatedPtrs * sizeof (KnownPtr));

	fillSerializedMemory();

	*nBytesP = m_nAllocSerializedBytes;
	return m_serializedMemory;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
void TreatmentRuleSerializer::fillSerializedMemory()
{
	int offset = 0;
	LangTreatmentRule *serializedRules = (LangTreatmentRule*)allocateSerializedMemory(m_nRules*sizeof(LangTreatmentRule), &offset);

	for (int i = 0; m_treatmentRules[i].triggerChar; i++)
	{
		int triggerCharOffset = 0;// serializeCharP(m_treatmentRules[i].triggerChar);
		int triggerStrOffset = serializeCharP(m_treatmentRules[i].triggerStr);
		int replacementOffset = serializeCharP(m_treatmentRules[i].replacementStr);

		serializedRules[i].triggerChar = m_treatmentRules[i].triggerChar;// (MYWCHAR *)triggerCharOffset;
		serializedRules[i].triggerStr		=  (MYWCHAR *) triggerStrOffset;
		serializedRules[i].replacementStr	=  (MYWCHAR *) replacementOffset;

		serializedRules[i].minNullMoves		= m_treatmentRules[i].minNullMoves;
		serializedRules[i].maxNullMoves		= m_treatmentRules[i].maxNullMoves;
		serializedRules[i].wholeWord		= m_treatmentRules[i].wholeWord;
		serializedRules[i].terminatedByOneSP= m_treatmentRules[i].terminatedByOneSP;
//		serializedRules[i].correctPrediction= m_treatmentRules[i].correctPrediction;
//		serializedRules[i].correctDisplay	= m_treatmentRules[i].correctDisplay;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int TreatmentRuleSerializer::deserializedPrint()
{
	for (int i = 0; m_treatmentRules[i].triggerChar; i++)
	{
	//	int offset = (int) m_treatmentRules[i].triggerChar;
		m_treatmentRules[i].triggerChar = m_treatmentRules[i].triggerChar;// (MYWCHAR *)&m_serializedMemory[offset];
		int offset = (int) m_treatmentRules[i].triggerStr;
		m_treatmentRules[i].triggerStr = (MYWCHAR *) &m_serializedMemory[offset];
		offset = (int) m_treatmentRules[i].replacementStr;
		m_treatmentRules[i].replacementStr = (MYWCHAR *) &m_serializedMemory[offset];
	}
	return 0;
}
