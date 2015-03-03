//#include "utility.h"
#include "dicttree.h"
#include "bucket.h"
#include <math.h>

const double sLog2val = log((double) 2);
double g_multiplyFactor = 0;
int    m_highestVal;
int    m_minimumVal = 0;

int calcBucketIdx(int preference);
void initializeLogValues(int lowestVal, int highestVal);

void calibrateLogValues(int lowestVal, int highestVal)
{
	int bucketIdx, j;

	m_minimumVal = 0;
	initializeLogValues(lowestVal, highestVal);
	m_minimumVal = lowestVal;

	for (j = 50; j < 1000; j += 50) 
	{
		bucketIdx = calcBucketIdx(j);
		printf("bucket %d for val %d \n",bucketIdx, j); 
	}

	for (j = 1000; j < 30000; j += 1000) 
	{
		bucketIdx = calcBucketIdx(j);
		printf("bucket %d for val %d \n",bucketIdx, j); 
	}

	for (j = 30000; j < 300000; j += 10000)
	{
		bucketIdx = calcBucketIdx(j);
		printf("bucket %d for val %d \n",bucketIdx, j); 
	}
}

void initializeLogValues(int lowestVal, int highestVal)
{
	unsigned bucketIdx = 0;
	int bucketIdxClosestToZero = 0xffff;
	double desiredMultiplyFactor = 0;

	m_highestVal = highestVal;
	g_multiplyFactor = 0;

	for (g_multiplyFactor = 1 ; g_multiplyFactor < 20; g_multiplyFactor++)
	{
		bucketIdx = calcBucketIdx(lowestVal);
		if (bucketIdx > 0)
		{
			if (bucketIdx > 1 && bucketIdx < bucketIdxClosestToZero)
			{
				bucketIdxClosestToZero = bucketIdx;
				desiredMultiplyFactor = g_multiplyFactor;
			}
		}
		else 
			break;
	}

	g_multiplyFactor = desiredMultiplyFactor;
	double endMultiplyFactorPlus1 = desiredMultiplyFactor + 1;

	bucketIdxClosestToZero = 0xffff;
	for (; g_multiplyFactor < endMultiplyFactorPlus1 ; g_multiplyFactor += (double) 0.1)
	{
		bucketIdx = calcBucketIdx(lowestVal);
		if (bucketIdx > 0)
		{
			if (bucketIdx > 1 && bucketIdx < bucketIdxClosestToZero)
			{
				bucketIdxClosestToZero = bucketIdx;
				desiredMultiplyFactor = g_multiplyFactor;
			}
		}
		else 
			break;
	}

	g_multiplyFactor = desiredMultiplyFactor;
}

int calcBucketIdx(int preference)
{
	if (preference == 0)
		return 0;

	if (preference <= m_minimumVal)
		return 1;

	double likelihood = (double) preference / (double) m_highestVal; 
	double logval = log(likelihood);
	logval = logval / sLog2val;
	logval = (double) 128 + (double) g_multiplyFactor * logval;
	int bucketidx = (int) logval;
	if (bucketidx <= 0)
		bucketidx = 1;
	return bucketidx;
}

static void setNormalizedPreferences(DictNode *startingNode)
{
	startingNode->Preference = calcBucketIdx(startingNode->Preference);
	if (startingNode->EndPointFlag)
	{
		startingNode->EndPreference = calcBucketIdx(startingNode->EndPreference);
	}

	for (DictNode *dNode = startingNode->CharacterList; dNode != NULL; dNode = dNode->Next)
		setNormalizedPreferences(dNode);
}

BOOL normalizeWithBuckets(DictNode *startingNode, DWORD lowest, DWORD highest)
{
	// make sure the range of values is between 0 and 128, even when little lists are processed
	calibrateLogValues(lowest, highest);
	setNormalizedPreferences(startingNode);
	return TRUE;
}

