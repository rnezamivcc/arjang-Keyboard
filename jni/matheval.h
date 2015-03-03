
#ifndef MATHEVAL_H
#define MATHEVAL_H

#include <float.h>

/* Some of you may choose to define TYPE as a "float" instead... */
#define TYPE            double          /* Type of numbers to work with */

#define VARLEN          30              /* Max length of variable names */
#define MAXVARS         50              /* Max user-defined variables */
#define TOKLEN          90              /* Max token length */

#define VAR             1
#define DEL             2
#define NUM             3

typedef struct
{
   WCHAR name[VARLEN + 1];               /* Variable name */
   TYPE value;                          /* Variable value */
} VARIABLE;

//typedef double (*fPtr)();
typedef struct
{
   WCHAR* name;                          /* Function name */
   int   args;                          /* Number of arguments to expect */
   TYPE  (*func)();                     /* Pointer to function */
} FUNCTION;

/* The following macros are ASCII Dependant, no EBCDIC here! */
#define iswhite(c)  (c == L' ' || c == L'\t')
#define isnumer(c)  ((c >= L'0' && c <= L'9') || c == L'.')
#define isalpha(c)  ((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9') \
                    || c == L'_')
#define isdelim(c)  (c == L'+' || c == L'-' || c == L'*' || c == L'/' || c == L'%' \
					|| c == L'�' || c == L'�' || c == 0x221A \
                    || c == L'^' || c == L'(' || c == L')' || c == L',' || c == L'=')

/* Codes returned from the evaluator */
#define E_OK           0        /* Successful evaluation */
#define E_SYNTAX       1        /* Syntax error */
#define E_UNBALAN      2        /* Unbalanced parenthesis */
#define E_DIVZERO      3        /* Attempted division by zero */
#define E_UNKNOWN      4        /* Reference to unknown variable */
#define E_MAXVARS      5        /* Maximum variables exceeded */
#define E_BADFUNC      6        /* Unrecognised function */
#define E_NUMARGS      7        /* Wrong number of arguments to funtion */
#define E_NOARG        8        /* Missing an argument to a funtion */
#define E_EMPTY        9        /* Empty expression */

#endif // MATHEVAL_H
