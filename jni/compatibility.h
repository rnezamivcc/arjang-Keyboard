#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#include "wltypes.h"


#define MAX_BONDING_CHARS 6

// operating systems
enum ePLATFORM
{
	eWORDLOGIC_WINDOWS	= 0,
	eWORDLOGIC_ANDROID	= 1,
	eWORDLOGIC_IOS		= 2
};

/////////////////////////////////////////////////////////////////////
// very important use (4 byte fields) to keep this long aligned !!
typedef struct TableSpace
{
	unsigned sizeOfEntry;
	void *tablePtr;
} TableSpace;

#define TBEVERBS			0
#define TBRULES				1
#define NTABLESPACES		2


extern TableSpace gTableSpaces[NTABLESPACES];

enum ePOSTAG
{
	ePOS_NOTAG = 0,
	ePOS_NC,				//common noun
	ePOS_NP,				//proper noun
	ePOS_NO,				//possessive noun
	ePOS_NM,				//mass noun (no plural or plural that changes the word's meaning)
	ePOS_NMO,				//possessive mass noun
	ePOS_NPO,				//possessive proper noun
	ePOS_NPS,				//plural proper noun
	ePOS_NS,				//plural common noun
	ePOS_NN,				//??
	ePOS_NVI,				//??
	ePOS_AB,				//abbreviation
	ePOS_ABO,				//possessive abbreviation
	ePOS_ABS,				//plural abbreviation		
	ePOS_ADJ,				//adjective
	ePOS_ADR,				//comparative adjective
	ePOS_ADT,				//superlative adjective
	ePOS_ADV,				//adverb
	ePOS_AJV,				//adjective-adverb (only appear in predicate, like aloud, aboard)
	ePOS_AP,				//proper adjective (capitalized) 
	ePOS_ART,				//article
	ePOS_AN,				//??
	ePOS_CF,				//combining form
	ePOS_CV,				//??
	ePOS_CNT,				//contraction
	ePOS_CON,				//conjunction
	ePOS_INT,				//nterjection or form of address
	ePOS_PP,				//preposition
	ePOS_PN,				//pronoun
	ePOS_PNO,				//possessive pronounc
	ePOS_PPT,				//present participle
	ePOS_PAT,				//past participle (not identical with past tense)
	ePOS_PRE,				//prefix
	ePOS_SYM,				//symbol
	ePOS_VA,				//auxiliary verb
	ePOS_VAS,				//??
	ePOS_VI,				//intransitive verb
	ePOS_VIP,				//past tense intransitive verb
	ePOS_VIS,				//3rd person singular, intransitive verb
	ePOS_VT,				//transitive verb
	ePOS_VTS,				//??
	ePOS_VTSS,				//??
	ePOS_VTP,				//past tense transitive verb
	ePOS_NW,				//nonword (slurred speech)
	ePOS_NEW_WORD,
	ePOS_HASHTAG,

	ePOS_MAXTAG = 0x40 // maximum 0x3F==63 tags are allowed. We keep the last bits for extra NLP tag to be used later!
};
////////////////////////////////////////////////////////////////////

// Fields should be all 4 byte aligned in size and address!
#define gCopyWrite "(C)Copyright 2000 WordLogic Corporation"

typedef struct DictHeader 
{
public:
	unsigned sizeTotal;	// total header size in bytes, including pointed-to strings like tablspace contents.
	intptr_t loadAddress; // loading address in memory at runtime. It gets adjusted at load time, if necessary.
	eEndianNess endianNess;
	eLanguage language;
	char copyright[((sizeof(gCopyWrite)+3)/4)*4];
	char createdTimeStr[40];
	char modifiedTimeStr[40];
	int buildNumber;    // it being incremented as new build of dictionary is available
	int versionNumber;   // gets incremented only when new data structure version or header structure is being deployed. So dictionaries with different versionNumber ARE NOT compatible.
	//BYTE    topLayerPref;   // all words with preferences higher than topLayerPref
							// are considered to be the top-layer of words and deserve
							// VIP treatment. (Whole words are stronger than suffixes.
							// Their position remains fixed so users can rely on fixed 
							// positions for these words.
	//int    cascadeThruPref;	// words with a preference lower than this the next cascading dictionaries are searched with 

	MYWCHAR  bondingChars[MAX_BONDING_CHARS];// these characters can connect two words to each other. Last one should be 0, to terminate the list!

	UINT	totalNumWords;
	UINT	totalNumChars;
	UINT	totalDataSize; // total size of the data after m_start_compact till the end of the last compactNode!
	TableSpace tableSpaces[NTABLESPACES]; // this should be at the end of DictHeader!!
} DictHeader;

enum eCompactNodeEncoding
{
	CODE_SUFFIX					= 0x01,	// indicates that this is a suffix
	CODE_ENDPOINT				= 0x02, // indicates there is an endpoint, so a word or a chunk 
	CODE_CAN_CHUNK		        = 0x04,	// this indicates that it can chunk, no space appended
	CODE_EVERB_ENDING			= 0x08, // indicates this is an everb ending node. 
	CODE_EVERB_ROOT				= 0x10,	// this indicates the root of the everb, for instance receiv on receive
	CODE_DYNAMIC				= 0x20,  // indicates this node been added dynamically. Used only for end nodes.
	CODE_POSEXT					= 0x40  // indicates this node has an extra POS tag, besides the one set in the node itself
};

#define CODE_SIZE				1    
#define EVERB_VARIDX_SIZE		1
#define COUNT_SIZE		        1
#define PTR_SIZE			    3
#define PREF_SIZE               1
#define POSTAG_SIZE				1

#define PIN_FOLLOWPTRS    0x0
#define PIN_PARENT		  0x1
#define PIN_ENDPOINT	  0x2
//#define PIN_STARTPOINT	  0x3
#define PIN_EVERB_VARIDX  0x3
#define PIN_POS_EXT		  0x4
#define PIN_WHOLE		  0x5
const int gNumLayouts = 6;

// defines for PhraseEngine node structure
#define PE_COMPACTNODE		0x0
#define PE_PARENT			0x1
#define PE_ENDPOINT			0x2
#define PE_NEXTWORDS		0x3
#define PE_NEXTPHRASES		0x4
#define PE_NEXTPHRASEPREFS	0x5

/*typedef struct NodeLayoutSpec 
{
	int code;
	int size;				
} NodeLayoutSpec;

const NodeLayoutSpec gNodeLayouts[gNumLayouts] = 
{
	{PIN_FOLLOWPTRS,		PTR_SIZE			}
,	{PIN_PARENT,			PTR_SIZE			}
,	{PIN_ENDPOINT,			PREF_SIZE			}
,	{PIN_NEXTPTRS,			PTR_SIZE			}
,	{PIN_NEXTPREFS,			PREF_SIZE			}
,	{PIN_EVERB_VARIDX,		EVERB_VARIDX_SIZE	}
};
*/
#endif // COMPATIBILITY_H
