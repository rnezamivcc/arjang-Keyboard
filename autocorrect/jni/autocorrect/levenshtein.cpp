#include "levenshtein.h"

#define LOG_TAG "levenshtein"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

int std::lev(const char* word1,const char* word2) {
	int len1, len2;

	len1 = strlen(word1);
	len2 = strlen(word2);

    int matrix[len1 + 1][len2 + 1];
    int i;

    for (i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    for (i = 0; i <= len2; i++) {
        matrix[0][i] = i;
    }
    for (i = 1; i <= len1; i++) {
        int j;
        char c1;

        c1 = word1[i-1];
        for (j = 1; j <= len2; j++) {
            char c2;

            c2 = word2[j-1];
            if (c1 == c2) {
                matrix[i][j] = matrix[i-1][j-1];
            }
            else {
                int deleteCost;
                int insert;
                int substitute;
                int minimum;

                deleteCost = matrix[i-1][j] + 1;
                insert = matrix[i][j-1] + 1;
                substitute = matrix[i-1][j-1] + 1;
                minimum = deleteCost;
                if (insert < minimum) {
                    minimum = insert;
                }
                if (substitute < minimum) {
                    minimum = substitute;
                }
                matrix[i][j] = minimum;
            }
        }
    }
    return matrix[len1][len2];
}

float std::keylev(const char* s1,const char* s2,kbdDistance* kbd) {
	int len1, len2;
	float deleteCost;
	float insertCost;
	float replaceCost;

	len1 = strlen(s1);
	len2 = strlen(s2);

    float matrix[len1 + 1][len2 + 1];
    int i;

    for (i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    for (i = 0; i <= len2; i++) {
        matrix[0][i] = i;
    }
    for (i = 1; i <= len1; i++) {
        int j;
        char c1;

        c1 = s1[i-1];
        for (j = 1; j <= len2; j++) {
            char c2;

            c2 = s2[j-1];
            if(c1 == c2) {
            	matrix[i][j] = matrix[i-1][j-1];
            } else if (kbd->dist(c1,c2) < 0.4) {
                matrix[i][j] = matrix[i-1][j-1] + 0.33333;
            } else {
                deleteCost = matrix[i-1][j] + 1;
                insertCost = matrix[i][j-1] + 1;
                replaceCost = matrix[i-1][j-1] + 1;

                matrix[i][j] = min(deleteCost,min(insertCost,replaceCost));
            }
        }
    }
    return matrix[len1][len2];
}
