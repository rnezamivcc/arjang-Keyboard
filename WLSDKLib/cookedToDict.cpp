// dicttree.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"

#include "dictmanager.h"
//#include "dictsyncer.h"
#include "everbserializer.h"
#include "treatmentruleserializer.h"


//#define BIGENGLISH_CONVERSION  1
//#define ENGLISH_CONVERSION  1
//#define FRENCH_CONVERSION   1
//#define MEDICAL_CONVERSION 1
//#define COMPAQ_TECHNICAL_CONVERSION 1
//#define USENGLISH_CONVERSION  1
//#define SMILEYS_CONVERSION 1
//#define PERSONAL_CONVERSION 1
//#define ADHOC_CONVERSION 1
//#define GERMAN_CONVERSION   1
//#define SPANISH_CONVERSION   1
//#define ITALIAN_CONVERSION   1
//#define PORTUGUESE_CONVERSION   1


#ifdef ENGLISH_CONVERSION
WCHAR *englishVariationsOnEVerbs1[] = {
	 L"ing"
	,L"ous"
	,L"ity"
	,L"um"
	,L"ation"
	,L"al"
	,L"ative"
	,L"able"
	,L"ance"
	,NULL
};

WCHAR *englishVariationsOnEVerbs2[] = {
	 L"sion"
	,L"ssion"
	,NULL
};

EVerbDefinition EnglishEVerbs[] = {
	{  L"e", 1, englishVariationsOnEVerbs1,  L"",   FALSE, TRUE  } // on any verb
	,{ L"de", 2, englishVariationsOnEVerbs2, L"",   FALSE, TRUE  } // on any verb
    ,{ NULL,  0,   NULL,					 L"",   FALSE, TRUE  } // on any verb
};

WCHAR EnglishBondingChars[] = L"";


// the last 3 fields (not mentioned)  are reserved for future use
//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord	  correctDisplay
LangTreatmentRule EnglishTreatmentRules[] =  {
    // change i into I
	{ L" ",	L"i",	L"I",	0, 0, 1, 1, FALSE,TRUE}
    // put the right double quotation to the end of previous word
   ,{ L" ",	L" \"",	L"\"",	1, 1, 0,1,FALSE,TRUE}
   ,{ L"",	L"",	L"",	0, 0, 0,0,FALSE,FALSE}
};




#endif


#ifdef FRENCH_CONVERSION

WCHAR *frenchVariationsOnERVerbs1[] = {
	 L"e"		,L"es"				,L"ons"		,L"ez"		,L"ent"		// indicatif present tense
	,L"ais"		,L"ait"				,L"ions"	,L"iez"		,L"aient"	// indicatif imparfait
	,L"ai"		,L"as"		,L"a"	,L"âmes"	,L"âtes"	,L"èrent"	// indicatif passe simple
	,L"erai"	,L"eras"	,L"era"	,L"erons"	,L"erez"	,L"eront"	// indicatif future tense 
	,L"e"		,L"es"				,L"ions"	,L"iez"		,L"ent"		// subjonctif present 
	,L"asse"	,L"asses"	,L"ât"	,L"assions"	,L"assiez"	,L"assent"	// subjonctif imparfait
	,L"e"		,L"ons"		,L"ez"										// imperatif present
	,L"erais"	,L"erait"			,L"erions"	,L"eriez"	,L"eraient"	// conditionell present
	,NULL
};

WCHAR *frenchVariationsOnIRVerbs2[] = {
	 L"is"		 ,L"it"				,L"issons"	,L"issez"	,L"issent"	// indicatif present tense
	,L"issais"	,L"issait"			,L"issions"	,L"issiez"	,L"issaient"// indicatif imparfait
	,L"is"		,L"it"				,L"îmes"	,L"îtes"	,L"irent"	// indicatif passe simple
	,L"irai"	,L"iras"	,L"ira"	,L"irons"	,L"irez"	,L"iront"	// indicatif future tense 
	,L"isse"	,L"isses"			,L"issions"	,L"issiez"	,L"issent"	// subjonctif present
	,L"isse"	,L"isses"	,L"ît"	,L"issions"	,L"issiez"	,L"issent"	// subjonctif imparfait
	,L"is"							,L"issons"	,L"issez"				// imperatif present
	,L"irais"	,L"irait"			,L"irions"	,L"iriez"	,L"iraient"	// conditionell present
	,NULL
};

WCHAR *frenchVariationsOnIRVerbs3a[] = {
	 L"s"		 // indicatif present tense
	,L"x"
	,L"t"
	,L"d"
	,L"ons"	,L"ez"	,L"ent"	,L"ont"

	,L"ais"		,L"ait"				,L"ions"	,L"iez"		,L"aient"	// indicatif imparfait
	,L"is"		,L"it"				,L"îmes"	,L"îtes"	,L"irent"	// indicatif passe simple
	,L"rai"		,L"ras"		,L"ra"	,L"rons"	,L"rez"		,L"ront"	// indicatif future simple 
	,L"e"		,L"es"				,L"ions"	,L"iez"		,L"ent"		// subjonctif present 
	,L"isse"	,L"isses"	,L"ît"	,L"issions"	,L"issiez"	,L"issent"	// subjonctif imparfait
	,L"s"							,L"ons"		,L"ez"					// imperatif present
	,L"rais"	,L"rait"			,L"rions"	,L"riez"	,L"raient"	// conditionell present
	,NULL
};

WCHAR *frenchVariationsOnIRVerbs3b[] = {
	 L"e"		,L"es"			,L"ons"		,L"ez"		,L"ent"		// indicatif present tense
	,L"ais"		,L"ait"			,L"ions"	,L"iez"		,L"aient"	// indicatif imparfait
	,L"us"		,L"ut"			,L"ûmes"	,L"ûtes"	,L"urent"	// indicatif passe simple
	,L"rai"		,L"ras"	,L"ra"	,L"rons"	,L"rez"		,L"ront"	// indicatif future simple 
	,L"e"		,L"es"			,L"ions"	,L"iez"		,L"ent"		// subjonctif present 
	,L"usse"	,L"usses",L"ût"	,L"ussions"	,L"ussiez"	,L"ussent"	// subjonctif imparfait
	,L"e"						,L"ons"		,L"ez"					// imperatif present
	,L"rais"	,L"rait"		,L"rions"	,L"riez"	,L"raient"	// conditionell present
	,NULL
};


WCHAR *frenchVariationsOnOIRVerbs3c[] = {
	 L"s"	,L"ds"	,L"t"	,L"ons"	,L"ez"	,L"ent"
	,L"ais"	,L"ait"	,L"ions"	,L"iez"	,L"aient"// indicatif imparfait
	,L"is"	,L"it"	,L"îmes"	,L"îtes",L"irent"// indicatif passe simple
	,NULL
};

WCHAR *frenchVariationsOnOIRVerbs3d[] = {
	 L"s"	,L"t"	,L"ons"		,L"ez"		,L"ent"
	,L"ais"	,L"ait"	,L"ions"	,L"iez"		,L"aient"	// indicatif imparfait
	,L"us"	,L"ut"	,L"ûmes"	,L"ûtes"	,L"urent"	// indicatif passe simple
	,NULL
};

WCHAR *frenchVariationsOnREVerbs3e[] = {
	 L"d"	,L"t"	,L"s"	,L"ds"	,L"ons"	,L"ez"	,L"ent"	,L"ont"
	,L"ais"	,L"ait"					,L"ions"	,L"iez"		,L"aient"	// indicatif imparfait
	,L"is"	,L"it"					,L"îmes"	,L"îtes"	,L"irent"	// indicatif passe simple
	,L"rai"	,L"ras"		,L"ra"		,L"rons"	,L"rez"		,L"ront"	// indicatif future simple 
	,L"e"	,L"es"					,L"ions"	,L"iez"		,L"ent"		// subjonctif present 
	,L"isse",L"isses"	,L"ît"		,L"issions"	,L"issiez"	,L"issent"	// subjonctif imparfait
	,L"s"							,L"ons"		,L"ez"					// imperatif present
	,L"rais",L"rait"				,L"rions"	,L"riez"	,L"raient"	// conditionell present
	,NULL
};

WCHAR *frenchVariationsOnREVerbs3f[] = {
	 L"e"		,L"es"				,L"ons"		,L"ez"		,L"ent"		// indicatif present tense
	,L"ais"		,L"ait"				,L"ions"	,L"iez"		,L"aient"	// indicatif imparfait
	,L"us"		,L"ut"				,L"ûmes"	,L"ûtes"	,L"urent"	// indicatif passe simple
	,L"rai"		,L"ras"		,L"ra"	,L"rons"	,L"rez"		,L"ront"	// indicatif future simple 
	,L"e"		,L"es"				,L"ions"	,L"iez"		,L"ent"		// subjonctif present 
	,L"usse"	,L"usses"	,L"ût"	,L"ussions"	,L"ussiez"	,L"ussent"	// subjonctif imparfait
	,L"e"							,L"ons"		,L"ez"					// imperatif present
	,L"rais"	,L"rait"			,L"rions"	,L"riez"	,L"raient"	// conditionell present
	,NULL
};



WCHAR *notOnSelectedERendings[] =  {
	 L"barber"	,L"barbier"	,L"berger"	,L"gaucher"	,L"messager"	,L"oliver"	,L"creiller"
	,L"portier"	,L"conner"	,L"usager"	,L"sentier"	,L"charter"	,L"palmer"	,L"passager"
	,L"plancher"	,L"léger"	,L"trier"	,L"millier"	,L"raider"	,L"horloger",L"oreiller"
	,L"charter"
	,NULL
};


WCHAR *notOnSelectedIRendings[] =  {
	 L"oir"	,L"plaisir"	,L"désir"	,L"éclair"	,L"chair"	,L"jouir"	,L"soupir"
	,NULL
};

WCHAR *selectedREEndings[] =  {
	 L"dre"	,L"ttre"	,L"uire"	,L"rire"	,L"dire"	,L"suffire"	,L"lire"	,L"dure"
	,L"lure"	,L"boire"	,L"faire"	,L"taire"	,L"raire"	,L"aître"	,L"inclure"
	,L"rompre"	,L"vivre"	,L"suivre"	,L"oire"	,L"convaincre"
	,NULL
};

WCHAR *notOnSelectedREendings[] =  {
	 L"sure"	,L"faire"	,L"ouvre"	,L"predure"	,L"peintre"	,L"pleure"	,L"cèdre"
	,L"perdure"	,L"notaire"	,L"commentaire"
	,NULL
};


WCHAR *selectedOIRendings[] =  {
	 L"cevoir"	,L"voir"	,L"loir"	,L"seoir"
	,NULL
};

WCHAR *notOnSelectedOIRendings[] =  {
	 L"réservoir"
	,NULL
};


EVerbDefinition FrenchEVerbs[] = {
	{ L"er", 2, frenchVariationsOnERVerbs1,	 L"",   FALSE, TRUE, NULL ,notOnSelectedERendings } // on any verb
   ,{ L"ir", 2, frenchVariationsOnIRVerbs2,	 L"o",   FALSE, TRUE, NULL , notOnSelectedIRendings } // on any verb ,no oir
   ,{ L"ir", 2, frenchVariationsOnIRVerbs3a,	 L"o",   FALSE, TRUE, NULL , notOnSelectedIRendings } // on any verb, no oir
   ,{ L"ir", 2, frenchVariationsOnIRVerbs3b,	 L"o",   FALSE, TRUE, NULL , notOnSelectedIRendings } // on any verb, no oir
   ,{ L"oir",3, frenchVariationsOnOIRVerbs3c,   L"",   FALSE, TRUE, selectedOIRendings , notOnSelectedOIRendings } // on any verb ,no oir
   ,{ L"oir",3, frenchVariationsOnOIRVerbs3d,   L"",   FALSE, TRUE, selectedOIRendings , notOnSelectedOIRendings } // on any verb ,no oir
   ,{ L"re", 2, frenchVariationsOnREVerbs3e,	 L"",   FALSE, TRUE, selectedREEndings, notOnSelectedREendings  } // on any verb
   ,{ L"re", 2, frenchVariationsOnREVerbs3f,	 L"",   FALSE, TRUE, selectedREEndings, notOnSelectedREendings  } // on any verb
   ,{ NULL,  0,   NULL,						 L"",   FALSE, TRUE, NULL, NULL  } // on any verb
};

WCHAR FrenchBondingChars[] = L"\'";


//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord  correctDisplay

LangTreatmentRule FrenchTreatmentRules[] =  {
   { L"\'",	L"e\'",	L"\'",	1, 1, 0,0,TRUE, FALSE}
  ,{ L"\'",	L"a\'",	L"\'",	1, 1, 0,0,TRUE, FALSE}
  // put the right double quotation to the end of previous word
  ,{ L" ",	L" \"",	L"\"",	1, 1, 0,1,FALSE,TRUE}
  ,{ L"",	L"",	L"",	0, 0, 0,0,FALSE,FALSE}
};

#endif


#ifdef GERMAN_CONVERSION
// fragen, frage,fragst, fragt, fragte, fragtest, fragte, fragten, fragest,fraget
WCHAR *germanVariationsOnEVerbs1[] = {
	 L"e"
	,L"st"
	,L"t"
	,L"te"
	,L"test"
	,L"ten"
	,L"est"
	,L"et"
	,NULL
};
// maesten, no Umlaut used.
// masten, maste ,mastest, mastet, mastete, mastetest, masteten, fragten, fragest,fraget
WCHAR *germanVariationsOnEVerbs2[] = {
	 L"e"
	,L"est"
	,L"et"
	,L"ete"
	,L"etest"
	,L"eten"
	,NULL
};

// handeln, handle ,handelst, handelt, handelte, handeltest, handelten, handeltet 
WCHAR *germanVariationsOnEVerbs3[] = {
	 L"le"
	,L"elst"
	,L"elt"
	,L"elte"
	,L"eltest"
	,L"elten"
	,NULL
};

WCHAR *eInsertionEndings[] =  {
	 L"chnen"
	,L"fnen"
	,L"gnen"
	,L"tmen"
	,L"den"
	,L"ten"
	,NULL
};

WCHAR *noEInsertionEndings[] =  {
	 L"rnen"
	,L"rmen"
	,L"lnen"
	,L"hnen"
	,L"lmen"
	,L"mmen"
	,L"nnen"
	,NULL
};

EVerbDefinition GermanEVerbs[] = {
    { L"en",	2, germanVariationsOnEVerbs1, L"",	FALSE, TRUE  , noEInsertionEndings, NULL } // on any verb
   ,{ L"en",	2, germanVariationsOnEVerbs2, L"",  FALSE, TRUE  , eInsertionEndings , NULL } // on any verb
   ,{ L"eln",	3, germanVariationsOnEVerbs3, L"",  FALSE, TRUE  , NULL  , NULL } // on any verb
   ,{ L"en",	2, germanVariationsOnEVerbs1, L"",	FALSE, TRUE  , NULL  , NULL } // on any verb
   ,{ NULL,     0,   NULL,				      L"",  FALSE, TRUE  , NULL  , NULL } // on any verb
};

WCHAR GermanBondingChars[] = L"";

//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord		   correctDisplay

LangTreatmentRule GermanTreatmentRules[] =  {
    { L"S",	L"SS",	L"ß",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"e",	L"ue",	L"ü",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"e",	L"oe",	L"ö",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"e",	L"ae",	L"ä",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"E",	L"UE",	L"Ü",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"E",	L"OE",	L"Ö",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"E",	L"AE",	L"Ä",	0, 2, 0,0,FALSE,TRUE}
   // put the right double quotation to the end of previous word
   ,{ L" ",	L" \"",	L"\"",	1, 1, 0,1,FALSE,TRUE}
   ,{ L"",	L"",	L"",	0, 0, 0,0,FALSE,FALSE}
};

#endif


#ifdef ITALIAN_CONVERSION
WCHAR *italianVariationsOnAREVerbs1[] = {
	 L"ando"
	,L"o"		,L"i"		,L"a"		,L"iamo"	,L"ate"		,L"ano"
	,L"avo"		,L"avi"		,L"ava"		,L"avamo"	,L"avate"	,L"avano"
	,L"ai"		,L"asti"	,L"ò"		,L"ammo"	,L"aste"	,L"arono"
	,L"erò"		,L"erai"	,L"erà"		,L"eremo"	,L"erete"	,L"eranno"
	,L"erei"	,L"eresti"	,L"erebbe"	,L"eremmo"	,L"ereste"	,L"erebbero"
													,L"iate"	,L"ino"	
	,L"assi"	,L"asse"				,L"assimo"	,L"aste"	,L"assero"
	,NULL
};

WCHAR *italianVariationsOnEREVerbs1[] = {
	 L"endo"
	,L"o"		,L"i"		,L"e"		,L"iamo"	,L"ete"		,L"ono"
	,L"evo"		,L"evi"		,L"eva"		,L"evamo"	,L"evate"	,L"evano"
	,L"ei"		,L"esti"	,L"è"		,L"emmo"	,L"este"	,L"erono"
	,L"erò"		,L"erai"	,L"erà"		,L"eremo"	,L"erete"	,L"eranno"
	,L"erei"	,L"eresti"	,L"erebbe"	,L"eremmo"	,L"ereste"	,L"erebbero"
													,L"iate"	,L"ano"	
	,L"essi"	,L"esse"				,L"essimo"	,L"este"	,L"essero"
	,NULL
};

WCHAR *italianVariationsOnIREVerbs1[] = {
	 L"endo"	
	,L"isco"	,L"isci"	,L"isce"	,L"iamo"	,L"ite"		,L"iscono"
	,L"ivo"		,L"ivi"		,L"iva"		,L"ivamo"	,L"ivate"	,L"ivano"
	,L"ii"		,L"isti"	,L"ì"		,L"immo"	,L"iste"	,L"irono"
	,L"irò"		,L"irai"	,L"irà"		,L"iremo"	,L"irete"	,L"iranno"
	,L"irei"	,L"iresti"	,L"irebbe"	,L"iremmo"	,L"ireste"	,L"irebbero"
	,L"isca"	,L"iscano"							,L"iate"
	,L"issi"	,L"isse"				,L"issimo"	,L"iste"	,L"issero"
	,NULL
};

WCHAR *italianVariationsOnIREVerbs2[] = {
	 L"endo"	
	,L"o"		,L"i"		,L"e"		,L"iamo"	,L"ite"		,L"ono"
	,L"ivo"		,L"ivi"		,L"iva"		,L"ivamo"	,L"ivate"	,L"ivano"
	,L"ii"		,L"isti"	,L"ì"		,L"immo"	,L"iste"	,L"irono"
	,L"irò"		,L"irai"	,L"irà"		,L"iremo"	,L"irete"	,L"iranno"
	,L"irei"	,L"iresti"	,L"irebbe"	,L"iremmo"	,L"ireste"	,L"irebbero"
	,L"ano"		,L"a"								,L"iate"
	,L"issi"	,L"isse"				,L"issimo"	,L"iste"	,L"issero"
	,NULL
};

WCHAR *selectedIREendings[] =  {
	 L"capire"
	,L"finire"
	,L"precepire"
	,L"colpire"
	,NULL
};

EVerbDefinition ItalianEVerbs[] = {
    { L"are",	3, italianVariationsOnAREVerbs1, L"",	FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ L"ere",	3, italianVariationsOnEREVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ L"ire",	3, italianVariationsOnIREVerbs1, L"",  FALSE, TRUE  , selectedIREendings , NULL  } // on any verb
   ,{ L"ire",	3, italianVariationsOnIREVerbs2, L"",	FALSE, TRUE  , NULL  , NULL  } // on any verb
   ,{ NULL,     0,   NULL,				      L"",  FALSE, TRUE  , NULL  , NULL  } // on any verb
};

WCHAR ItalianBondingChars[] = L"\'";

//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord	  correctDisplay

LangTreatmentRule ItalianTreatmentRules[] =  {
   { L"\'",	L"della\'",	L"dell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"dello\'",	L"dell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"bella\'",	L"bell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"bello\'",	L"bell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"dalla\'",	L"dall\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"dallo\'",	L"dall\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"nella\'",	L"nell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"nello\'",	L"nell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"quella\'",L"quell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"quello\'",L"quell\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"questa\'",L"quest\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"questo\'",L"quest\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"sullo\'" ,L"sull\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"sulla\'" ,L"sull\'",	1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"ci\'",	L"c\'",		1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"di\'",	L"d\'",		1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"la\'",	L"l\'",		1, 1, 1,0,TRUE, FALSE}
  ,{ L"\'",	L"lo\'",	L"l\'",		1, 1, 1,0,TRUE, FALSE}
  // put the right double quotation to the end of previous word
  ,{ L" ",	L" \"",	L"\"",		    1, 1, 0,1,FALSE,TRUE}

#if 0 // debug stuff , put every rule possible in one dictionary to test everything
  ,{ L"a",	L"¨a",	L"ä",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"S",	L"SS",	L"ß",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"e",	L"ue",	L"ü",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"e",	L"oe",	L"ö",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"e",	L"ae",	L"ä",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"E",	L"UE",	L"Ü",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"E",	L"OE",	L"Ö",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"E",	L"AE",	L"Ä",	0, 2, 0,0,FALSE,TRUE}
   ,{ L"-",	L"--",	L" —",	0, 2, 0,0,FALSE,TRUE}
   ,{ L" ",	L" —",	L"—",	1, 1, 0,1,FALSE,TRUE}
#endif
  ,{ L"",	L"",	L"",	0, 0, FALSE,FALSE,FALSE,FALSE,FALSE, FALSE,	FALSE}
};

#endif


#ifdef SPANISH_CONVERSION
WCHAR *spanishVariationsOnARVerbs1[] = {
	 L"o"		,L"as"		,L"a"	,L"amos"	,L"áis"		,L"an"
	,L"aba"		,L"abas"			,L"ábamos"	,L"abais"	,L"aban"
	,L"é"		,L"aste"	,L"ó"				,L"asteis"	,L"aron"
	,L"aré"		,L"arás"	,L"ará"	,L"aremos"	,L"éis"		,L"arán"
	,L"aría"	,L"aríais"			,L"aríamos"	,L"arían"	,L"arías"
	,L"e"							,L"emos"	,L"ad"		,L"en"
	,L"es"		,L"éis"
	,L"ara"		,L"aras"			,L"áramos"	,L"arías"	,L"aran"
				,L"aréis"	,L"aría",L"aríais"	,L"aríamos"
	,L"are"		,L"ares"			,L"áremos"	,L"areis"	,L"aren"
	,NULL
};

WCHAR *spanishVariationsOnERVerbs1[] = {
	 L"o"	,L"es"		,L"e"		,L"emos"	,L"éis"		,L"en"
	,L"ía"	,L"íais"				,L"íamos"	,L"ías"		,L"ían"
	,L"í"	,L"iste"	,L"ió"		,L"imos"	,L"isteis"	,L"irías"
	,L"ieron"
	,L"eré"	,L"erás"				,L"eremos"	,L"eréis"	,L"erán"
	,L"ería",L"ías"					,L"eríamos"	,L"eríais"	,L"erían"
	,L"e"	,L"a"					,L"amos"	,L"ed"		,L"an"
	,L"as"	,L"áis"		,L"a"	
	,L"iera",L"ieras"				,L"iéramos"	,L"ierais"	,L"ieran"
	,L"iere",L"ieres"				,L"iéremos"	,L"iereis"	,L"ieren"
	,NULL
};


WCHAR *spanishVariationsOnIRVerbs1[] = {
	 L"o"	,L"es"		,L"e"		,L"í"	,L"imos"	,L"ís"		,L"en"
	,L"ía"	,L"ías"							,L"íamos"	,L"íais"	,L"ían"
	,L"í"	,L"iste"	,L"ió"				,L"imos"	,L"isteis"	,L"ieron"
	,L"iré"	,L"irás"	,L"irá"				,L"iremos"	,L"iréis"	,L"irán"
	,L"iría",L"irías"						,L"iríamos"	,L"iríais"	,L"irían"
	,L"a"									,L"amos"	,L"id"		,L"an"
	,L"a"	,L"as"							,L"amos"	,L"áis"
	,L"iera",L"ieras"						,L"iéramos"	,L"ierais"	,L"ieran"
	,L"iere",L"ieres"						,L"iéremos"	,L"iereis"	,L"ieren"
	,NULL
};

EVerbDefinition SpanishEVerbs[] = {
    { L"ar",	2, spanishVariationsOnARVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ L"er",	2, spanishVariationsOnERVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ L"ir",	2, spanishVariationsOnIRVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ NULL,     0,   NULL,				         L"",  FALSE, TRUE  , NULL, NULL } // on any verb
};

WCHAR SpanishBondingChars[] = L"";

//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord		    correctDisplay

LangTreatmentRule SpanishTreatmentRules[] =  {
  // changes to short hyphens with a long dash sign 
	{ L"-",	L"--",	L" —",		0, 2, 0,0,FALSE,TRUE}
  // put the right long dash sign to the end of previous word
  ,{ L" ",	L" —",	L"—",		1, 1, 0,1,FALSE,TRUE}
  // put the right double quotation to the end of previous word
  ,{ L" ",	L" \"",	L"\"",		1, 1, 0,1,FALSE,TRUE}
  // in the future apostrophe quote will become a dumb charact just like double qoute for span
  ,{ L" ",	L" \'",	L"\'",		1, 1, 0,1,FALSE,TRUE}
  ,{ L"",	L"",	L"",		0, 0, 0,0,FALSE,FALSE}
};
#endif

#ifdef PORTUGUESE_CONVERSION
WCHAR *portugueseVariationsOnARVerbs1[] = {
	 L"o"		,L"as"		,L"a"		,L"amos"	,L"ais"		,L"am"		// present tense
	,L"ava"		,L"avas"				,L"ávamos"	,L"aveis"	,L"avam"	// Pretirito imperfeito
	,L"ei"		,L"aste"	,L"ou"					,L"astes"	,L"aram"	// Pretirito perfeito	
	,L"ado"																	// perfeito composito
	,L"ara"		,L"aras"				,L"áremos"	,L"áreis"				// mais-que-perfeito
	,L"arei"	,L"arás"	,L"ará"		,L"aremos"	,L"areis"	,L"arão"	// futuro presente	
	,L"aria"	,L"arias"				,L"aríamos"	,L"áríeis"	,L"ariam"	// futuro do preterito
	,L"e"		,L"es"					,L"emos"	,L"eis"		,L"em"		// subjonctivo presente
	,L"asse"	,L"asses"				,L"ássemos"	,L"ásseis"	,L"assem"	// subjonctivo imperfeito
	,L"ar"		,L"ares"				,L"armos"	,L"ardes"	,L"arem"	// subjonctivo futuro
				,L"ai"														// imperativo
	,NULL
};

WCHAR *portugueseVariationsOnERVerbs1[] = {
	 L"o"		,L"es"		,L"e"		,L"emos"	,L"eis"		,L"em"		// present tense
	,L"ia"		,L"ias"					,L"íamos"	,L"íeis"	,L"iam"	// Pretirito imperfeito
	,L"i"		,L"este"	,L"eu"					,L"estes"	,L"eram"	// Pretirito perfeito	
	,L"ido"																	// perfeito composito
	,L"era"		,L"eras"				,L"êramos"	,L"êreis"				// mais-que-perfeito
	,L"erei"	,L"erás"	,L"erá"		,L"eremos"	,L"ereis"	,L"erão"	// futuro presente	
	,L"eria"	,L"erias"				,L"eríamos"	,L"eríeis"	,L"eriam"	// futuro do preterito
	,L"a"		,L"as"					,L"amos"	,L"ais"		,L"am"		// subjonctivo presente
	,L"esse"	,L"esses"				,L"êssemos"	,L"êsseis"	,L"essem"	// subjonctivo imperfeito
	,L"er"		,L"eres"				,L"ermos"	,L"erdes"	,L"erem"	// subjonctivo futuro
				,L"ei"														// imperativo
	,NULL
};

WCHAR *portugueseVariationsOnIRVerbs1[] = {
	 L"o"		,L"es"		,L"e"		,L"imos"	,L"is"		,L"em"		// present tense
	,L"ia"		,L"ias"					,L"íamos"	,L"íeis"	,L"iam"		// Pretirito imperfeito
	,L"i"		,L"iste"	,L"iu"		,L"imos"	,L"istes"	,L"iram"	// Pretirito perfeito	
	,L"ido"																	// perfeito composito
	,L"ira"		,L"iras"				,L"íramos"	,L"íreis"				// mais-que-perfeito
	,L"irei"	,L"irás"	,L"irá"		,L"iremos"	,L"ireis"	,L"irão"	// futuro presente	
	,L"iria"	,L"irias"				,L"iríamos"	,L"iríeis"	,L"iriam"	// futuro do preterito
	,L"a"		,L"as"					,L"amos"	,L"ais"		,L"am"		// subjonctivo presente
	,L"isse"	,L"isses"				,L"íssemos"	,L"ísseis"	,L"issem"	// subjonctivo imperfeito
	,L"ir"		,L"ires"				,L"irmos"	,L"irdes"	,L"irem"	// subjonctivo futuro
				,L"i"														// imperativo
	,NULL
};

EVerbDefinition PortugueseEVerbs[] = {
    { L"ar",	2, portugueseVariationsOnARVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ L"er",	2, portugueseVariationsOnERVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ L"ir",	2, portugueseVariationsOnIRVerbs1, L"",  FALSE, TRUE  , NULL, NULL } // on any verb
   ,{ NULL,     0,   NULL,				           L"",  FALSE, TRUE  , NULL, NULL } // on any verb
};

WCHAR PortugueseBondingChars[] = L"";

//triggerChar replacementStr maxNullMovements terminatedByOneSpace onHardLetter	        correctPrediction
//     triggerStr  minNullMovements     wholeWord			onSoftLetter   correctDisplay

LangTreatmentRule PortugueseTreatmentRules[] =  {
  // changes to short hyphens with a long dash sign 
	{ L"-",	L"--",	L" —",		0, 2, 0,0,FALSE,TRUE}
  // put the right long dash sign to the end of previous word
  ,{ L" ",	L" —",	L"—",		1, 1, 0,1,FALSE,TRUE}
  // put the right double quotation to the end of previous word
  ,{ L" ",	L" \"",	L"\"",		1, 1, 0,1,FALSE,TRUE}
  // in the future apostrophe quote will become a dumb charact just like double qoute for span
  ,{ L" ",	L" \'",	L"\'",		1, 1, 0,1,FALSE,TRUE}
  ,{ L"",	L"",	L"",		0, 0, 0,0,FALSE,FALSE}
};
#endif

void convertTextToDict()
{
	CDictionaryTree *dictionaryTree =  NULL;
	int nSerializedBytes = 0;

	CDictManager *dictManager = new CDictManager();
	dictManager->fillActiveConfiguration();

	dictionaryTree =  new CDictionaryTree();
	EVerbSerializer *eVerbSerializer = new EVerbSerializer();
	TreatmentRuleSerializer *treatmentRuleSerializer = new TreatmentRuleSerializer();

#ifdef ENGLISH_CONVERSION
	dictionaryTree->setTopLayerPref(100000);
	dictionaryTree->processDictFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\englishcorpus.cooked.txt"), 
		"spi",  WORDSIDE_DICTIONARY, NULL,1,0, FALSE);	// string, preference, ignore field
#endif
#ifdef BIGENGLISH_CONVERSION 
	dictionaryTree->setTopLayerPref(32000);
	dictionaryTree->processDictFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\englishcorpus.cooked.txt"), 
		"spi",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif
#ifdef USENGLISH_CONVERSION 
	dictionaryTree->setTopLayerPref(32000);
	dictionaryTree->processDictFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\usenglishcorpus.cooked.txt"), 
		"spi",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif

#ifdef FRENCH_CONVERSION
//	dictionaryTree->setTopLayerPref(6000); // first thousand words
//	dictionaryTree->processDictFile(TEXT("d:\\temp\\dev\\wlsdk\\Dictionaries\\Cooked\\lemonde.cooked.txt"), 
//		"spi",  WORDSIDE_DICTIONARY, NULL,1,50,FALSE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\FrenchCorpus.cooked.txt"), 
		"spi", WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\FrenchNames.txt"), 
		"spi", WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field

#endif

#ifdef SPANISH_CONVERSION
//	dictionaryTree->setTopLayerPref(200); // first thousand words
#if 0
	dictionaryTree->processDictFile(TEXT("C:\\MYprojects\\wlsdk\\Dictionaries\\Cooked\\Spanish1.txt"), 
		"ps",  WORDSIDE_DICTIONARY, NULL,1,4,FALSE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\MYprojects\\wlsdk\\Dictionaries\\Cooked\\Spanish2.txt"), 
		"ps",  WORDSIDE_DICTIONARY, NULL,1,4,FALSE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\MYprojects\\wlsdk\\Dictionaries\\Cooked\\Spanish3.txt"), 
		"ps",  WORDSIDE_DICTIONARY, NULL,1,4,FALSE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\MYprojects\\wlsdk\\Dictionaries\\Cooked\\Spanish4.txt"), 
		"ps",  WORDSIDE_DICTIONARY, NULL,1,4,FALSE);	// string, preference, ignore field
#endif
	dictionaryTree->processDictFile(TEXT("C:\\MYprojects\\wlsdk\\Dictionaries\\Cooked\\Spanishnovels.txt"), 
		"ps",  WORDSIDE_DICTIONARY, NULL,1,2,FALSE);	// string, preference, ignore field

#endif

#ifdef GERMAN_CONVERSION
	dictionaryTree->setTopLayerPref(500);
	dictionaryTree->processDictFile(TEXT("C:\\MY CODE\\wlsdk\\Dictionaries\\Cooked\\GermanCorpus.cooked.txt"), "spi",  
		WORDSIDE_DICTIONARY, NULL,1,0,TRUE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\MY CODE\\wlsdk\\Dictionaries\\Cooked\\GermanNameCorpus.cooked.txt"), "spi",  
		WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif

#ifdef ITALIAN_CONVERSION
	dictionaryTree->setTopLayerPref(3000); // first 2300 words 
//	dictionaryTree->processDictFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\italianwords.txt"), "spi",  
//		WORDSIDE_DICTIONARY, NULL,1,80,TRUE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\italianwords.txt"), "spi",  
		WORDSIDE_DICTIONARY, NULL,1,80,TRUE);	// string, preference, ignore field
	dictionaryTree->processDictFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\italiannames.txt"), "spi",  
		WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif

#ifdef PORTUGUESE_CONVERSION
	dictionaryTree->setTopLayerPref(5000);	// first 3000 words
	dictionaryTree->processDictFile(
		TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\portuguese\\words.txt"), "spi",  
		WORDSIDE_DICTIONARY, NULL,1,150,FALSE);	// string, preference, ignore field
	dictionaryTree->processDictFile(
		TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\portuguese\\names.txt"), "spi",  
		WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif


#ifdef PERSONAL_CONVERSION
	dictionaryTree->processDictFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\personal.cooked.txt"), 
		"spi",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif
#ifdef MEDICAL_CONVERSION
	dictionaryTree->processDictFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\medicalcorpus.cooked.txt"), 
		"spi",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif
#ifdef COMPAQ_TECHNICAL_CONVERSION
	dictionaryTree->processDictFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\compaqtechnical.cooked.txt"), 
		"spi",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, ignore field
#endif
#ifdef SMILEYS_CONVERSION
	dictionaryTree->processDictFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\smileys.cooked.txt"), 
		"spd",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, description field
#endif
#ifdef ADHOC_CONVERSION
//	dictionaryTree->processDictFile(TEXT("d:\\temp\\dev\\chat1and2.txt"), 
//		"spd",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, description field
//	dictionaryTree->processArticle(TEXT("d:\\temp\\dev\\chat1and2.txt"), (Dictionary *) NULL, 1);
//	dictionaryTree->processDictFile(TEXT("d:\\temp\\dev\\alphachat.txt"), 
//		"spd",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, description field
	dictionaryTree->processDictFile(TEXT("d:\\temp\\dev\\chat1and2korrigiert.txt"), 
		"spd",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, description field
//	dictionaryTree->processArticle(TEXT("d:\\temp\\dev\\gastebuch.txt"), (Dictionary *) NULL, 1);
//	dictionaryTree->processArticle(TEXT("d:\\temp\\dev\\gastebuch.txt"), (Dictionary *) NULL, 1);
//	dictionaryTree->processArticle(TEXT("d:\\temp\\dev\\gastebuch.txt"), (Dictionary *) NULL, 1);
//	dictionaryTree->processArticle(TEXT("d:\\temp\\dev\\gastebuch.txt"), (Dictionary *) NULL, 1);
#endif

//	dictionaryTree->processDictFile(SMILEYS_TXT, "spd",  WORDSIDE_DICTIONARY, NULL,1,0,FALSE);	// string, preference, description field
//	dictionaryTree->processDictFile(SMALLABC_TXT, "spd", OBJECTSIDE_DICTIONARY | WORDSIDE_DICTIONARY, NULL,1,9,FALSE);	// string, preference, description till the eol


	dictionaryTree->collectSuffixes();

#ifdef ENGLISH_CONVERSION
//	dictionaryTree->locateEVerbs(EnglishEVerbs, TRUE);
	dictionaryTree->locateEVerbs(EnglishEVerbs, FALSE);
//	void *serializedData = eVerbSerializer->initalize(EnglishEVerbs, &nSerializedBytes);
//	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	void *serializedData = treatmentRuleSerializer->initalize(EnglishTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(EnglishBondingChars);

#endif

#ifdef SPANISH_CONVERSION
	dictionaryTree->locateEVerbs(SpanishEVerbs, FALSE);
//	void *serializedData = eVerbSerializer->initalize(SpanishEVerbs, &nSerializedBytes);
//	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	void *serializedData = treatmentRuleSerializer->initalize(SpanishTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(SpanishBondingChars);
#endif

#ifdef FRENCH_CONVERSION
	dictionaryTree->locateEVerbs(FrenchEVerbs, FALSE);
//	void *serializedData = eVerbSerializer->initalize(FrenchEVerbs, &nSerializedBytes);
//	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	void *serializedData = treatmentRuleSerializer->initalize(FrenchTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(FrenchBondingChars);
	dictionaryTree->printTreeNodes(TEXT("c:\\verbroots.txt"), OnlyVerbRoots);

#endif

#ifdef GERMAN_CONVERSION
	dictionaryTree->locateEVerbs(GermanEVerbs, TRUE);
	void *serializedData = eVerbSerializer->initalize(GermanEVerbs, &nSerializedBytes);
	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	serializedData = treatmentRuleSerializer->initalize(GermanTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(GermanBondingChars);
#endif

#ifdef ITALIAN_CONVERSION
	dictionaryTree->locateEVerbs(ItalianEVerbs, TRUE);
	void *serializedData = eVerbSerializer->initalize(ItalianEVerbs, &nSerializedBytes);
	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	serializedData = treatmentRuleSerializer->initalize(ItalianTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(ItalianBondingChars);
#endif

#ifdef PORTUGUESE_CONVERSION
	dictionaryTree->locateEVerbs(PortugueseEVerbs, TRUE);
	void *serializedData = eVerbSerializer->initalize(PortugueseEVerbs, &nSerializedBytes);
	dictionaryTree->setSerializedEVerbsData(serializedData, nSerializedBytes);
	serializedData = treatmentRuleSerializer->initalize(PortugueseTreatmentRules, &nSerializedBytes);
	dictionaryTree->setSerializedRuleData(serializedData, nSerializedBytes);
	dictionaryTree->setBondingChars(PortugueseBondingChars);
#endif

	dictionaryTree->normalizePreferences();

	
//	dictionaryTree->printTree(createFullPathFileName(TEXT("normtree.txt")));
//	dictionaryTree->printTreeSort((TEXT("d:\\temp\\dev\\germannorm2000.txt")));
//	dictionaryTree->printTreeASort(createFullPathFileName(TEXT("normtree.txt")));
//	dictionaryTree->printTop2000Words();

	if (dictionaryTree->compactTree(NULL))
	{

		dictionaryTree->linkObjectsInCompactTree(NULL);

#ifdef ENGLISH_CONVERSION
//		dictionaryTree->setLanguageSpecificSwitches(&englishSwitches);
		dictionaryTree->compactToFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Build\\English.dict"));
#endif
#ifdef BIGENGLISH_CONVERSION
//		dictionaryTree->setLanguageSpecificSwitches(&englishSwitches);
		dictionaryTree->compactToFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\bigenglish.dict"));
#endif
#ifdef USENGLISH_CONVERSION
//		dictionaryTree->setLanguageSpecificSwitches(&englishSwitches);
		dictionaryTree->compactToFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\usenglish.dict"));
#endif
#ifdef FRENCH_CONVERSION
//		dictionaryTree->setLanguageSpecificSwitches(&frenchSwitches);
		dictionaryTree->compactToFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Build\\French.dict"));
#endif
#ifdef SPANISH_CONVERSION
		dictionaryTree->compactToFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Build\\Spanish.dict"));
#endif

#ifdef PORTUGUESE_CONVERSION
		dictionaryTree->compactToFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Build\\Portuguese.dict"));
#endif

#ifdef ITALIAN_CONVERSION
		dictionaryTree->compactToFile(TEXT("C:\\myprojects\\wlsdk\\Dictionaries\\Build\\Italian.dict"));
#endif
#ifdef GERMAN_CONVERSION
//		dictionaryTree->setLanguageSpecificSwitches(&frenchSwitches);
		dictionaryTree->printTreeSort(TEXT("C:\\MY CODE\\wlsdk\\Dictionaries\\Build\\german.alpha.txt"));
		dictionaryTree->compactToFile(TEXT("C:\\MY CODE\\wlsdk\\Dictionaries\\Build\\German.dict"));
#endif
#ifdef PERSONAL_CONVERSION
		dictionaryTree->compactToFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\personal.dict"));
#endif
#ifdef MEDICAL_CONVERSION
		dictionaryTree->compactToFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\medical.dict"));
#endif
#ifdef SMILEYS_CONVERSION
		dictionaryTree->compactToFile(TEXT("d:\\myprojects\\wlsdk\\Dictionaries\\Cooked\\smileys.dict"));
#endif
#ifdef ADHOC_CONVERSION
		dictionaryTree->compactToFile(TEXT("d:\\temp\\dev\\adhoc.dict"));
#endif
	}
	delete dictionaryTree;
}

int main(int argc, char* argv[])
{
	void convertTextToDict();

	convertTextToDict();

	return 0;
}


