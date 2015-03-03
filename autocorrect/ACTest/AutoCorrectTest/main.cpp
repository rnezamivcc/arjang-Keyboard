#include "..//..//AutocorrectTwo//jni//utility.h"
#include "..//..//AutocorrectTwo//jni//acGraph.h"
#include "..//..//AutocorrectTwo//jni//acMatch.h"
#include "..//..//AutocorrectTwo//jni//simpleVector.h"

MYWCHAR* randomword() 
{
	char ret[10];
	
	int l = 1 + (rand() % 10);
	for(int c = 0;c < l;c++)
	{
		ret[c] = (char)('a' + (rand()%26));
	}

	return toW(ret);
}

int main() 
{
	FILE* graphfile = fopen("english.out", "r");
	if(graphfile == NULL) 
	{
		printf("Failed opening graph file english.out.\n");
		return 0;
	}

	acGraph* graph = new acGraph(graphfile);
	fclose(graphfile);

	for(int c = 0; c < 100; c++) 
	{
		MYWCHAR *s = randomword();
		printf("Trying matches for %s: \n", s);

		SimpleVector<acMatch> sug = graph->autocorrect(s, NULL, 10);
		int nsug = sug.size();

		for(int cb = 0; cb < nsug; cb++)
			printf("%s\n", toA(sug[cb].match));

		printf("\n");
	}


}