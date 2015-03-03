#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "wltypes.h"

typedef struct knownPtr 
{
	GENPTR ptrVal;
	unsigned offset;
} KnownPtr;

class Serializer
{
public:
	Serializer();
	~Serializer();

	char   *allocateSerializedMemory(int nBytes, int *offset = NULL);

	void	increaseHandledPtrs(GENPTR ptr, int offset);
	int		serializeCharP(WCHAR *str);
	int		serializeCharPP(WCHAR **strp);
	int		ptrAlreadyStored(GENPTR ptr);

	char	*m_serializedMemory;
	int		m_nAllocSerializedBytes;
	int		m_nUsedSerializedBytes;

	KnownPtr *m_handledPtrs;	// collection of ptr which were already stored (serialized) 
	int		m_nHandledPtrs;
	int		m_nAllocatedPtrs;
};

#endif   //SERIALIZER_H

