
#include "StdAfx.h"
#include "everbserializer.h"
#include "compactstore.h"
#include "utility.h"

//////////////////////////////////////////////////////////////////////////////////////////////
EVerbSerializer::EVerbSerializer() 
{
	m_EVerbs = NULL;
	m_nEVerbDefs = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
void *EVerbSerializer::initialize(EVerbDefinition *langEVerbDefinitions, int *nBytesP)
{
	m_EVerbs = langEVerbDefinitions;
	int nChars = 0;
	for (int i = 0; m_EVerbs[i].triggers[0]!=NUL; i++)
	{
		MYWCHAR **variations = m_EVerbs[i].variationsOnEVerbs;
		for (int j = 0; variations[j]; j++)
		{
			nChars += (mywcslen(variations[j]) + 1);
			m_nAllocatedPtrs++; // pointer for this variation
		}
		m_nAllocatedPtrs++; // last pointer added is NULL, to terminate the above list of variations.
		m_nEVerbDefs++;
	}
	m_nEVerbDefs++; // this is for the last everb =NULL so that we can finish the list.
	// two pointer in every EVerbDefinition, make sure long aligned as well
//	m_nAllocatedPtrs++; //last null pointer to finish the whole everbs in memory
	m_nAllocSerializedBytes = m_nAllocatedPtrs*sizeof(GENPTR) + nChars*sizeof(MYWCHAR) + m_nEVerbDefs*sizeof(EVerbDefinitionR);

	m_nAllocSerializedBytes = (m_nAllocSerializedBytes + 0x3) & ~0x3;
	m_serializedMemory = (char *) calloc(1, m_nAllocSerializedBytes);
	assert(m_serializedMemory != NULL);
		
	m_handledPtrs = (KnownPtr *)calloc(1, m_nAllocatedPtrs * sizeof(KnownPtr));
	fillSerializedMemory();

	*nBytesP = m_nAllocSerializedBytes;
	return m_serializedMemory;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void EVerbSerializer::fillSerializedMemory()
{
	int offset = 0;
	int everbArraySize = m_nEVerbDefs * sizeof(EVerbDefinitionR);
	EVerbDefinitionR *dEVerbs = (EVerbDefinitionR *)allocateSerializedMemory(everbArraySize, &offset);

	for (int i = 0; m_EVerbs[i].triggers[0]!=NUL; i++)
	{
		dEVerbs[i].triggers[0] = m_EVerbs[i].triggers[0]; 
		dEVerbs[i].triggers[1] = m_EVerbs[i].triggers[1]; 
		dEVerbs[i].triggers[2] = m_EVerbs[i].triggers[2]; 
		dEVerbs[i].triggers[3] = m_EVerbs[i].triggers[3];
		dEVerbs[i].variationsOnEVerbs = (MYWCHAR **)serializeCharPP(m_EVerbs[i].variationsOnEVerbs);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
int EVerbSerializer::deserializedPrint()
{
	for (int i = 0; m_EVerbs[i].triggers[0] != NUL; i++)
	{
		int offset = (int) m_EVerbs[i].triggers;
		m_EVerbs[i].triggers[0] = ((MYWCHAR *)&m_serializedMemory[offset])[0];
		m_EVerbs[i].triggers[1] = ((MYWCHAR *)&m_serializedMemory[offset])[1];
		m_EVerbs[i].triggers[2] = ((MYWCHAR *)&m_serializedMemory[offset])[2];
		m_EVerbs[i].triggers[3] = ((MYWCHAR *)&m_serializedMemory[offset])[3];
		offset = (int)m_EVerbs[i].variationsOnEVerbs;
		m_EVerbs[i].variationsOnEVerbs = (MYWCHAR **) &m_serializedMemory[offset];

		MYWCHAR **variations = m_EVerbs[i].variationsOnEVerbs;
		for (int j = 0; variations[j]; j++)
		{
			offset = (int) variations[j];
			variations[j] = (MYWCHAR *) &m_serializedMemory[offset];
		}
	}
	return 0;
}
