/*************************************************************************
**                                                                       **
** MATHEVAL.C   Expression Evaluator                                     **
**                                                                       **
** AUTHOR:      Mark Morley                                              **
** COPYRIGHT:   (c) 1992 by Mark Morley                                  **
** DATE:        December 1991                                            **
** HISTORY:     Jan 1992 - Made it squash all command line arguments     **
**                         into one big long string.                     **
**                       - It now can set/get VMS symbols as if they     **
**                         were variables.                               **
**                       - Changed max variable name length from 5 to 15 **
**              Jun 1992 - Updated comments and docs                     **
**                                                                       **
** You are free to incorporate this code into your own works, even if it **
** is a commercial application.  However, you may not charge anyone else **
** for the use of this code!  If you intend to distribute your code,     **
** I'd appreciate it if you left this message intact.  I'd like to       **
** receive credit wherever it is appropriate.  Thanks!                   **
**                                                                       **
** I don't promise that this code does what you think it does...         **
**                                                                       **
** Please mail any bug reports/fixes/enhancments to me at:               **
**      morley@camosun.bc.ca                                             **
** or                                                                    **
**      Mark Morley                                                      **
**      3889 Mildred Street                                              **
**      Victoria, BC  Canada                                             **
**      V8Z 7G1                                                          **
**      (604) 479-7861                                                   **
**                                                                       **
 *************************************************************************/
#if 0
/* #define VAX */             /* Uncomment this line if you're using VMS */
#include <wtypes.h>

#include "matheval.h"
#include <math.h>
#include <setjmp.h>
#include "utility.h"

#define ERR(n) {myERROR=n; ERPOS=(int) ((WCHAR *)expression-ERANC-1); wcscpy(ERTOK,pktoken); longjmp(jb,1);}

/* These defines only happen if the values are not already defined!  You may
   want to add more precision here, if your machine supports it. */
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_E
#define M_E     2.71828182845904523536028747135266249775724709369995
#endif


/*************************************************************************
**                                                                       **
** PROTOTYPES FOR CUSTOM MATH FUNCTIONS                                  **
**                                                                       **
 *************************************************************************/

double deg( double x );
double rad( double x );
double mysin(double arg);
double mycos(double arg);
double mytan(double arg);
double fac(double x);

/*************************************************************************
**                                                                       **
** VARIABLE DECLARATIONS                                                 **
**                                                                       **
 *************************************************************************/
int   myERROR;                  /* The error number */
WCHAR  ERTOK[TOKLEN + 1];     /* The token that generated the error */
int   ERPOS;                 /* The offset from the start of the expression */
WCHAR * ERANC;                 /* Used to calculate ERPOS */

/*
   Add any "constants" here...  These are "read-only" values that are
   provided as a convenience to the user.  Their values can not be
   permanently changed.  The first field is the variable name, the second
   is its value.
*/
VARIABLE Consts[] =
{
   /* name, value */
   { L"pi",      M_PI },
   { L"e",       M_E },
   { 0 }
};

/*
   Add any math functions that you wish to recognize here...  The first
   field is the name of the function as it would appear in an expression.
   The second field tells how many arguments to expect.  The third is
   a pointer to the actual function to use.
*/
FUNCTION Funcs[] =
{
   /* name, function to call */
  // { L"log",     1,    (double (__cdecl *)(void)) log },
  // { L"log10",   1,    (double (__cdecl *)(void)) log10 },
  // { L"sqrt",    1,    (double (__cdecl *)(void)) sqrt },
  // { L"floor",   1,    (double (__cdecl *)(void)) floor },
//   { "cos",     1,    (double (__cdecl *)(void)) cos },
   { L"cos",     1,    (double (__cdecl *)(void)) mycos },
   { L"sin",     1,    (double (__cdecl *)(void))mysin },
   { L"tan",     1,    (double (__cdecl *)(void))mytan },
//    { L"acos",    1,    (double (__cdecl *)(void))acos },
//    { L"asin",    1,    (double (__cdecl *)(void))asin },
//    { L"atan",    1,    (double (__cdecl *)(void))atan },
//    { L"cosh",    1,    (double (__cdecl *)(void))cosh },
//    { L"sinh",    1,    (double (__cdecl *)(void))sinh },
//    { L"tanh",    1,    (double (__cdecl *)(void))tanh },
//    { L"exp",     1,    (double (__cdecl *)(void)) exp },
//    { L"ceil",    1,    (double (__cdecl *)(void))ceil },
//    { L"abs",     1,    (double (__cdecl *)(void))fabs },
//    { L"hypot",   2,    (double (__cdecl *)(void))hypot },
//    { L"pow",     2,    (double (__cdecl *)(void))pow },
//    { L"deg",     1,    (double (__cdecl *)(void))deg },
//    { L"rad",     1,    (double (__cdecl *)(void))rad },
//    { L"fac",     1,    (double (__cdecl *)(void))fac },
   { 0 }
};

VARIABLE        Vars[MAXVARS];       /* Array for user-defined variables */
WCHAR*  expression;          /* Pointer to the user's expression */
WCHAR   pktoken[TOKLEN + 1];   /* Holds the current token */
int             type;                /* Type of the current token */
jmp_buf         jb;                  /* jmp_buf for errors */

bool g_bDegreesSelected = true;

int Level1( TYPE* r );
void Level2( TYPE* r );
void Level3( TYPE* r );
void Level4( TYPE* r );
void Level5( TYPE* r );
void Level6( TYPE* r );


/*************************************************************************
**                                                                       **
** Some custom math functions...   Note that they must be prototyped     **
** above (if your compiler requires it)                                  **
**                                                                       **
** deg( x )             Converts x radians to degrees.                   **
** rad( x )             Converts x degrees to radians.                   **
**                                                                       **
 *************************************************************************/

double mywtof(WCHAR *wstr)
{
	char astr[200];

	memset(astr, 0, sizeof(astr));
	wcstombs(astr,wstr, wcslen(wstr));
	return atof(astr);
}


double mysin(double arg)
{
	double radianArg = arg;

	if (g_bDegreesSelected)
	{
		radianArg = (arg * M_PI) / 180;
//		radianArg = rad(arg); 
	}
	return sin(radianArg);
}

double mycos(double arg)
{
	double radianArg = arg;
	if (g_bDegreesSelected)
	{
		radianArg = (arg * M_PI) / 180;
//		radianArg = rad(arg); 
	}
	return cos(radianArg);
}

double mytan(double arg)
{
	double radianArg = arg;
	if (g_bDegreesSelected)
	{
		radianArg = (arg * M_PI) / 180;
//		radianArg = rad(arg); 
	}
	return tan(radianArg);
}

double fac(double x)
{
	double i = floor(x);
	double res = 0;

	if (i > 1)
	{
		res = i;
		i--;
		for (; i>1; i--)
			res = res * i;
	}
	return res;
}

double deg( double x )
{
   return( x * 180.0 / M_PI );
}

double rad( double x )
{
   return( x * M_PI / 180.0 );
}

/*************************************************************************
**                                                                       **
** GetSymbol( char* s )                                                  **
**                                                                       **
** This routine obtains a value from the program's environment.          **
** This works for DOS and VMS (and other OS's???)
**                                                                       **
 ************************************************************************/

int GetSymbol( WCHAR* s, TYPE* v )
{
   WCHAR* e;

   e = _wgetenv( s );
   if( !e )
      return( 0 );
   *v = mywtof( e );
   return( 1 );
}

/*************************************************************************
**                                                                       **
** strlwr( char* s )   Internal use only                                 **
**                                                                       **
** This routine converts a string to lowercase.  I know many compilers   **
** offer their own routine, but my VMS compiler didn't so...             **
** Again, this one is ASCII specific!                                    **
**                                                                       **
 *************************************************************************/

#if 0
static
void strlwr( char* s )
{
   while( *s )
   {
      if( *s >= 'A' && *s <= 'Z' )
         *s += 32;
      s++;
   }
}

#endif

/*************************************************************************
**                                                                       **
** ClearAllVars()                                                        **
**                                                                       **
** Erases all user-defined variables from memory. Note that constants    **
** can not be erased or modified in any way by the user.                 **
**                                                                       **
** Returns nothing.                                                      **
**                                                                       **
 *************************************************************************/

void ClearAllVars()
{
   int i;

   for( i = 0; i < MAXVARS; i++ )
   {
      *Vars[i].name = 0;
      Vars[i].value = 0;
   }
}

/*************************************************************************
**                                                                       **
** ClearVar( char* name )                                                **
**                                                                       **
** Erases the user-defined variable that is called NAME from memory.     **
** Note that constants are not affected.                                 **
**                                                                       **
** Returns 1 if the variable was found and erased, or 0 if it didn't     **
** exist.                                                                **
**                                                                       **
 *************************************************************************/

int ClearVar( WCHAR* name )
{
   int i;

   for( i = 0; i < MAXVARS; i++ )
      if( *Vars[i].name && ! wcscmp( name, Vars[i].name ) )
      {
         *Vars[i].name = 0;
         Vars[i].value = 0;
         return( 1 );
      }
   return( 0 );
}

/*************************************************************************
**                                                                       **
** GetValue( char* name, TYPE* value )                                   **
**                                                                       **
** Looks up the specified variable (or constant) known as NAME and       **
** returns its contents in VALUE.                                        **
**                                                                       **
** First the user-defined variables are searched, then the constants are **
** searched.                                                             **
**                                                                       **
** Returns 1 if the value was found, or 0 if it wasn't.                  **
**                                                                       **
 *************************************************************************/

int GetValue( WCHAR* name, TYPE* value )
{
   int i;

   /* First check for an environment variable reference... */
   if( *name == '_' )
      return( GetSymbol( name + 1, value ) );

   /* Now check the user-defined variables. */
   for( i = 0; i < MAXVARS; i++ )
   {
      if( *Vars[i].name && ! wcscmp( name, Vars[i].name ) )
      {
         *value = Vars[i].value;
         return( 1 );
      }
   }

   /* Now check the programmer-defined constants. */
   for( i = 0; Consts[i].name && *Consts[i].name; i++ )
   {
      if( *Consts[i].name && ! wcscmp( name, Consts[i].name ) )
      {
         *value = Consts[i].value;
         return( 1 );
      }
   }
   return( 0 );
}

/*************************************************************************
**                                                                       **
** SetValue( char* name, TYPE* value )                                   **
**                                                                       **
** First, it erases any user-defined variable that is called NAME.  Then **
** it creates a new variable called NAME and gives it the value VALUE.   **
**                                                                       **
** Returns 1 if the value was added, or 0 if there was no more room.     **
**                                                                       **
 *************************************************************************/

int SetValue( WCHAR* name, TYPE* value )
{
   int  i;

   ClearVar( name );
   for( i = 0; i < MAXVARS; i++ )
      if( ! *Vars[i].name )
      {
         wcscpy( Vars[i].name, name );
         Vars[i].name[VARLEN] = 0;
         Vars[i].value = *value;
         return( 1 );
      }
   return( 0 );
}

void PrintVars( WCHAR *text )
{
   ShowInfo("%s \n", toA(text));
 
   for( int i = 0; i < MAXVARS; i++ )
   {
      if(*Vars[i].name )
		   ShowInfo("%s : %d\n", toA(Vars[i].name), (int) Vars[i].value);
   }
}

/*************************************************************************
**                                                                       **
** Parse()   Internal use only                                           **
**                                                                       **
** This function is used to grab the next token from the expression that **
** is being evaluated.                                                   **
**                                                                       **
 *************************************************************************/

static void Parse()
{
   WCHAR* t;

   type = 0;
   t = (WCHAR *) &pktoken[0];
   while( iswhite( *expression ) )
      expression++;
   if( isdelim( *expression ) )
   {
      type = DEL;
      *t++ = *expression++;
   }
   else if( isnumer( *expression ) )
   {
      type = NUM;
      while( isnumer( *expression ) )
         *t++ = *expression++;
   }
   else if( isalpha( *expression ) )
   {
      type = VAR;
      while( isalpha( *expression ) )
        *t++ = *expression++;
      pktoken[VARLEN] = 0;
   }
   else if( *expression )
   {
      *t++ = *expression++;
      *t = 0;
	  {
		  myERROR=E_SYNTAX; 
		  ERPOS=(int) ((WCHAR *)expression-ERANC-1); 
		  wcscpy(ERTOK,pktoken);
		  longjmp(jb,1);
	  }

//      ERR( E_SYNTAX );
   }
   *t = 0;
   while( iswhite( *expression ) )
      expression++;
}

/*************************************************************************
**                                                                       **
** Level1( TYPE* r )   Internal use only                                 **
**                                                                       **
** This function handles any variable assignment operations.             **
** It returns a value of 1 if it is a top-level assignment operation,    **
** otherwise it returns 0                                                **
**                                                                       **
 *************************************************************************/

static int Level1( TYPE* r )
{
   WCHAR t[VARLEN + 1];

   if( type == VAR )
      if( *expression == L'=' )
      {
         wcscpy( t, pktoken );
         Parse();
         Parse();
         if( !*pktoken )
         {
            ClearVar( t );
            return(1);
         }
         Level2( r );
         if( ! SetValue( t, r ) )
            ERR( E_MAXVARS );
         return( 1 );
      }
   Level2( r );
   return( 0 );
}


/*************************************************************************
**                                                                       **
** Level2( TYPE* r )   Internal use only                                 **
**                                                                       **
** This function handles any addition and subtraction operations.        **
**                                                                       **
 *************************************************************************/

static void Level2( TYPE* r )
{
   TYPE t = 0;
   WCHAR o;

   Level3( r );
   while( (o = *pktoken) == L'+' || o == L'-' )
   {
      Parse();
      Level3( &t );
      if( o == L'+' )
         *r = *r + t;
      else if( o == L'-' )
         *r = *r - t;
   }
}


/*************************************************************************
**                                                                       **
** Level3( TYPE* r )   Internal use only                                 **
**                                                                       **
** This function handles any multiplication, division, or modulo.        **
**                                                                       **
 *************************************************************************/

static void Level3( TYPE* r )
{
   TYPE t;
   WCHAR o;

   Level4( r );
   while( (o = *pktoken) == L'*' || o == L'/' || o == L'%' )
   {
      Parse();
      Level4( &t );
      if( o == L'*' )
         *r = *r * t;
      else if( o == L'/' )
      {
         if( t == 0 )
            ERR( E_DIVZERO );
         *r = *r / t;
      }
      else if( o == L'%' )
      {
         if( t == 0 )
            ERR( E_DIVZERO );
         *r = fmod( *r, t );
      }
   }
}

/*************************************************************************
**                                                                       **
** Level4( TYPE* r )   Internal use only                                 **
**                                                                       **
** This function handles any "to the power of" operations.               **
**                                                                       **
 *************************************************************************/

static void Level4( TYPE* r )
{
   TYPE t;

   Level5( r );
   if( *pktoken == L'²' )
   {
      *r = pow( *r, 2 );
   }
   else if( *pktoken == L'³' )
   {
      *r = pow( *r, 3 );
   }
   else if( *pktoken == L'^' )
   {
      Parse();
      Level5( &t );
      *r = pow( *r, t );
   }
}


/*************************************************************************
**                                                                       **
** Level5( TYPE* r )   Internal use only                                 **
**                                                                       **
** This function handles any unary + or - signs.                         **
**                                                                       **
 *************************************************************************/

static void Level5( TYPE* r )
{
   WCHAR o = 0;

   if( *pktoken == L'+' || *pktoken == L'-' || *pktoken == 0x221A )
   {
      o = *pktoken;
      Parse();
   }
   Level6( r );
   if( o == L'-' )
      *r = -*r;
   else if (o == 0x221A) // square root sign
	  *r = sqrt(*r);
}


/*************************************************************************
**                                                                       **
** Level6( TYPE* r )   Internal use only                                 **
**                                                                       **
** This function handles any literal numbers, variables, or functions.   **
**                                                                       **
 *************************************************************************/

static void Level6( TYPE* r )
{
   int  i;
   int  n;
   TYPE a[3];

   if( *pktoken == L'(' )
   {
      Parse();
      if( *pktoken == L')' )
         ERR( E_NOARG );
      Level1( r );
      if( *pktoken != L')' )
         ERR( E_UNBALAN );
      Parse();
   }
   else
   {
      if( type == NUM )
      {
         *r = (TYPE) mywtof( pktoken );
         Parse();
      }
      else if( type == VAR )
      {
         if( *expression == L'(' )
         {
            for( i = 0; Funcs[i].name && *Funcs[i].name; i++ )
			{
               if( ! wcscmp( pktoken, Funcs[i].name ) )
               {
                  Parse();
                  n = 0;
                  do
                  {
                     Parse();
                     if( *pktoken == L')' || *pktoken == L',' )
                        ERR( E_NOARG );
                     a[n] = 0;
                     Level1( &a[n] );
                     n++;
                  } while( n < 4 && *pktoken == L',' );
                  Parse();
                  if( n != Funcs[i].args )
                  {
                     wcscpy( pktoken, Funcs[i].name );
                     ERR( E_NUMARGS );
                  }
				  if (Funcs[i].args == 1)
				  {
					*r =  ((double (__cdecl *) (double)) Funcs[i].func)( a[0]);
				  }
				  else if (Funcs[i].args == 2)
				  {
					*r =  ((double (__cdecl *) (double, double)) Funcs[i].func)( a[0], a[1]);
				  }
                  return;
               }
			}
//               if( ! *Funcs[i].name )
            ERR( E_BADFUNC );
         }
         else if( ! GetValue( pktoken, r ) )
           ERR( E_UNKNOWN );
         Parse();
      }
      else
         ERR( E_SYNTAX );
   }
}

/*************************************************************************
**                                                                       **
** Evaluate( char* e, TYPE* result, int* a )                             **
**                                                                       **
** This function is called to evaluate the expression E and return the   **
** answer in RESULT.  If the expression was a top-level assignment, a    **
** value of 1 will be returned in A, otherwise it will contain 0.        **
**                                                                       **
** Returns E_OK if the expression is valid, or an error code.            **
**                                                                       **
 *************************************************************************/

int Evaluate( WCHAR* e, TYPE* result, int* a )
{
   if( setjmp( jb ) )
      return( myERROR );
   expression =  e;
   ERANC = e;
   _wcslwr( expression );
   *result = 0;
   Parse();
   if( ! *pktoken )
      ERR( E_EMPTY );
   /* don't allow assignments like 5=10 */
   if (type == NUM && *expression == L'=')
	   ERR( E_SYNTAX );
   *a = Level1( result );
   return( E_OK );
}

int PKEvaluate(WCHAR* wExpression, double *result, int* a)
{
	int   ec;

	/* Call the evaluator. */
	if( (ec = Evaluate( wExpression, (TYPE*) result, a )) == E_OK )
	{
		return 1;
	}
	else
		return 0;
}



#if 0

/*************************************************************************
**                                                                       **
** What follows is a main() routine that evaluates the command line      **
** arguments one at a time, and displays the results of each expression. **
** Without arguments, it becomes an interactive calculator.              **
**                                                                       **
 *************************************************************************/

#include <stdio.h>

char* ErrMsgs[] =
{
   "Syntax error",
   "Unbalanced parenthesis",
   "Division by zero",
   "Unknown variable",
   "Maximum variables exceeded",
   "Unrecognized function",
   "Wrong number of arguments to function",
   "Missing an argument",
   "Empty expression"
};

main( int argc, char* argv[] )
{
   TYPE  result;
   int   i;
   int   ec;
   int   a;
   char  line[1024];
   char* v;

   ClearAllVars();
   /* If we have command line arguments then we evaluate them and exit. */
   if( argc > 1 )
   {
      /* Concatenate all arguments into one string. */
      strcpy( line, argv[1] );
      for( i = 2; i < argc; i++ )
         strcat( line, argv[i] );

      /* Call the evaluator. */
      if( (ec = Evaluate( line, &result, &a )) == E_OK )
      {
         /* If we didn't assign a variable, then print the result. */
         if( ! a )
            printf( "%g\n", result );
      }
      else if( ec != E_EMPTY )
      {
         /* Display error info.  In this example, an E_EMPTY error is ignored. */
         printf( "ERROR: %s - %s", ErrMsgs[myERROR - 1], ERTOK );
         printf( "\n%s", ERANC );
         printf( "\n%*s^\n", ERPOS, "" );
      }
      return;
   }

   /* There were no command line arguments, so lets go interactive. */
   printf( "\nEE - Equation Evaluator" );
   printf( "\nBy Mark Morley  December 1991" );
   printf( "\nEnter EXIT to quit.\n" );
   printf( "\nEE> " );

   /* Input one line at a time from the user.  Note that it uses stdin, so on
      DOS or UNIX you could pipe a list of expressions into it... */
   for( gets( line ); !feof( stdin ); gets( line ) )
   {
      strlwr( line );

      /* Did the user ask to exit? */
      if( ! strcmp( line, "exit" ) )
         return;

      /* Did the user ask to see the variables in memory? */
      else if( ! strcmp( line, "vars" ) )
      {
         for( i = 0; i < MAXVARS; i++ )
            if( *Vars[i].name )
               printf( "%s = %g\n", Vars[i].name, Vars[i].value );
      }

      /* Did the user ask to see the constants in memory? */
      else if( ! strcmp( line, "cons" ) )
      {
         for( i = 0; Consts[i].name && *Consts[i].name; i++ )
            printf( "%s = %g\n", Consts[i].name, Consts[i].value );
      }

      /* Did the user ask to clear all variables? */
      else if( ! strcmp( line, "clr" ) )
         ClearAllVars();

      /* If none of the above, then we attempt to evaluate the user's input. */
      else
      {
         /* Call the evaluator. */
         if( (ec = Evaluate( line, &result, &a )) == E_OK )
         {
            /* Only display the result if it was not an assignment. */
            if( ! a )
               printf( "%g\n", result );
         }
         else if( ec != E_EMPTY )
         {
            /* Display error information.  E_EMPTY is ignored. */
            printf( "ERROR: %s - %s", ErrMsgs[myERROR - 1], ERTOK );
            printf( "\n%s", ERANC );
            printf( "\n%*s^\n", ERPOS, "" );
         }
      }
      printf( "EE> " );
   }
}

#endif

#endif // #if 0