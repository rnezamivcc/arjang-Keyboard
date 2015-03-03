#ifndef TREATMENTRULESERIALIZER_H
#define TREATMENTRULESERIALIZER_H

#include "dicttree.h"
#include "serializer.h"

#include "compactstore.h"
class TreatmentRuleSerializer: public Serializer
{
public:
	TreatmentRuleSerializer();

	void	*initialize(LangTreatmentRule *langTreatmentRules, int *nBytesP);

private:

	LangTreatmentRule		*m_treatmentRules;
	int						m_nRules;

	void	fillSerializedMemory();
	int		deserializedPrint();

};

#endif   //TREATMENTRULESERIALIZER_H

