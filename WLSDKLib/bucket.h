#ifndef BUCKET_H
#define BUCKET_H

typedef struct Bucket
{
	DWORD beginVal; // included
	DWORD endVal;	// excluded
	int   cnt;
} Bucket;

#define NBUCKETS 127 

#endif BUCKET_H

