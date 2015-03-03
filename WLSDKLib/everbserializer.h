#ifndef EVERBSERIALIZER_H
#define EVERBSERIALIZER_H

#include "dicttree.h"
#include "serializer.h"

class EVerbSerializer: public Serializer
{
  
public:
	EVerbSerializer();

	void	*initialize(EVerbDefinition *langEVerbDefinitions, int *nBytesP);

private:

	EVerbDefinition		*m_EVerbs;
	int					m_nEVerbDefs;

	void fillSerializedMemory();
	int deserializedPrint();
};

#endif   //EVERBSERIALIZER_H

