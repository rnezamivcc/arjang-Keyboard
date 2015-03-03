
#include "StdAfx.h"
#include "serializer.h"
#include "utility.h"

Serializer::Serializer() 
{
	m_serializedMemory = NULL;
	m_nAllocSerializedBytes = 0;
	m_nUsedSerializedBytes = 0;
	m_handledPtrs = NULL;
	m_nHandledPtrs = 0;
	m_nAllocatedPtrs = 0;
}

Serializer::~Serializer() 
{
	if (m_serializedMemory) 
	{
		free( m_serializedMemory);
		m_serializedMemory = NULL;
		m_nUsedSerializedBytes = 0;
		m_nAllocSerializedBytes = 0;
	}

	if (m_handledPtrs) 
	{
		free( m_handledPtrs);
		m_handledPtrs = NULL;
		m_nHandledPtrs = 0;
		m_nAllocatedPtrs = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
int Serializer::ptrAlreadyStored(GENPTR ptr)
{
	for (int i = 0; i < m_nHandledPtrs; i++)
	{
		if (m_handledPtrs[i].ptrVal == ptr)
			return m_handledPtrs[i].offset;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////
void Serializer::increaseHandledPtrs(GENPTR ptr, int offset)
{
	m_handledPtrs[m_nHandledPtrs].ptrVal = ptr;
	m_handledPtrs[m_nHandledPtrs++].offset = offset;
}

////////////////////////////////////////////////////////////////////////////////////////////
int Serializer::serializeCharP(MYWCHAR *str)
{
	int strLen = mywcslen(str);

	int offset = ptrAlreadyStored((GENPTR)str);
	if (offset != -1)
		return offset;

	MYWCHAR *serializedP = (MYWCHAR *)allocateSerializedMemory((strLen+1)*sizeof(MYWCHAR), &offset);
	for (int i = 0 ; i < strLen;i++)
		serializedP[i] = str[i];
	serializedP[strLen] = NUL;
	increaseHandledPtrs((GENPTR)str, offset);

	return offset;
}

//////////////////////////////////////////////////////////////////////////////////////////////
int Serializer::serializeCharPP(MYWCHAR **strp)
{
	int offset = ptrAlreadyStored((GENPTR)strp);
	if (offset != -1)
		return offset;

	int j;
	for (j = 0; strp[j]; j++);

	intptr_t *serializedPP = (intptr_t *)allocateSerializedMemory((j + 1)*sizeof(int*), &offset);
	//int *serializedPP = (int *) &m_serializedMemory[strpoffset];

	for (j = 0; strp[j]; j++)
		serializedPP[j] = serializeCharP(strp[j]);

	increaseHandledPtrs((GENPTR) strp, offset);
	return offset;
}

///////////////////////////////////////////////////////////////////////////////////////////
char *Serializer::allocateSerializedMemory(int nBytes, int *curOffset)
{
	*curOffset = m_nUsedSerializedBytes;
	m_nUsedSerializedBytes += nBytes;
	assert(m_nUsedSerializedBytes <= m_nAllocSerializedBytes);
	return &m_serializedMemory[*curOffset];
}

