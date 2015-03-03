#include "acstdafx.h"
#include "personalgraph.h"

personalGraph::personalGraph() {
	verts.push_back(personalVertex(0));
}

personalGraph::personalGraph(const char *buf,int size) {
	int nshorts;
	const short* sp;
	const int* ip;
	int nverts;
	int c, cc;
	int nchildren;

	sp = (const short*)buf;
	sp += 2;
	ip = (const int*)ip;
	nshorts = (size - 4) / 2;
	nverts = *ip;

	verts = vector<personalVertex>(nverts); // initialize the vertex vector to hold all of the vertices that we are about to load

	for(c = 0;c < nverts;c++) {
		verts[c].ch = *sp++;
		nchildren = *sp++;

		for(cc = 0;cc < nchildren;cc++) {
			verts[c].children.push_back(&(verts[*sp++]));
		}
	}
}

/*****
* write
*	Writes the personal auto-correct dictionary to a buffer, in the following format:
*		- one 32-bit integer describing the number of vertices in the graph
*		- a series of graph vertices represented as two shorts ch and nc where
*			ch is the UCS-16 character for the vertex and nc is the number of children it has
*			in the DAWG. This is followed by nc more shorts representing the indices of the children
*			of the current vertex in the buffer. For example, the first vertex has an index of 0 and the second vertex
*			has an index of 1.
*
*		Finally, it's important that the root vertex in the graph be assigned index 0.
*
*	size	-	a pointer to an integer which will be set to the size of the buffer
*
*	Jonathon Simister (January, 2013)
******/
char* personalGraph::write(int* size) {
	int nlinks;
	int c, cc;
	short* ret;
	short* sp;
	int bufsize;
	int* ip;

	nlinks = 0;

	for(c = 0;c < verts.size();c++) {
		nlinks += verts[c].children.size();
		verts[c].ord = c;
	}

	bufsize = nlinks + verts.size()*2 + 4;

	ret = new short[bufsize];

	ip = (int*)ret;

	*ip = verts.size();

	sp = ret + 2;

	for(c = 0;c < verts.size();c++) {
		*sp++ = verts[c].ch;
		*sp++ = verts[c].children.size();

		for(cc = 0;cc < verts[c].children.size();cc++) {
			*sp++ = verts[c].children[cc]->ord;
		}
	}

	*size = bufsize;

	return (char*)ret;
}
