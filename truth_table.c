#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_EXPRESSION 1000
#define MAX_VARIABLES 20

/* ============================================================
   GLOBAL VARIABLES
   ============================================================ */

char expression[MAX_EXPRESSION];


/* Stores the variables detected in the expression */
char variables[MAX_VARIABLES];


int variableCount = 0;


/*
   Stores the current truth value of every alphabet variable.

   Example:

   values['P' - 'A'] = 1;
   values['Q' - 'A'] = 0;
*/
int values[26];


/* Current position of the parser in the expression */
int position = 0;


/* Error handling */
int parseError = 0;
char errorMessage[200];


/* ============================================================
   ERROR HANDLING
   ============================================================ */

void setError(const char *message)
{
    if (!parseError)
    {
        parseError = 1;

        strncpy(errorMessage, message, sizeof(errorMessage) - 1);

        errorMessage[sizeof(errorMessage) - 1] = '\0';
    }
}


/* ============================================================
   SPACE HANDLING
   ============================================================ */

void skipSpaces(void)
{
    while (isspace((unsigned char)expression[position]))
    {
        position++;
    }
}


/* ============================================================
   CASE-INSENSITIVE WORD COMPARISON
   ============================================================ */

int wordEqualsIgnoreCase(const char *word, int length, const char *keyword)
{
    int i;

    if ((int)strlen(keyword) != length)
    {
        return 0;
    }

    for (i = 0; i < length; i++)
    {
        if (toupper((unsigned char)word[i]) != keyword[i])
        {
            return 0;
        }
    }

    return 1;
}


/* ============================================================
   ADD VARIABLE
   ============================================================ */

void addVariable(char variable)
{
    int i;


    /* Check whether variable already exists */

    for (i = 0; i < variableCount; i++)
    {
        if (variables[i] == variable)
        {
            return;
        }
    }


    /* Check maximum number of variables */

    if (variableCount >= MAX_VARIABLES)
    {
        setError("Too many distinct variables for this program.");

        return;
    }


    /* Add new variable */

    variables[variableCount] = variable;

    variableCount++;
}


/* ============================================================
   DETECT VARIABLES
   ============================================================ */

int detectVariables(void)
{
    int i = 0;

    variableCount = 0;

    parseError = 0;

    errorMessage[0] = '\0';

    while (expression[i] != '\0')
    {
        /*
           If alphabetic characters are found,
           read the complete word.
        */

        if (isalpha((unsigned char)expression[i]))
        {
            char word[100];

            int length = 0;

            while (isalpha((unsigned char)expression[i]))
            {
                if (length < 99)
                {
                    word[length] = expression[i];

                    length++;
                }

                i++;
            }


            word[length] = '\0';


            /*
               Ignore logical operators written as words.
            */

            if (
                    wordEqualsIgnoreCase(word, length, "AND")
                    ||
                    wordEqualsIgnoreCase(word, length, "OR")
                    ||
                    wordEqualsIgnoreCase(word, length, "NOT")
                )
            {
                continue;
            }


            /*
               A variable must currently be a single
               uppercase alphabetic letter.
            */

            if (length == 1 && isupper((unsigned char)word[0]))
            {
                addVariable(word[0]);


                if (parseError)
                {
                    return 0;
                }
            }
            else
            {
                setError(
                    "Variables must be single uppercase letters. "
                    "Use AND, OR and NOT as operators."
                );

                return 0;
            }
        }
        else
        {
            i++;
        }
    }


    if (variableCount == 0)
    {
        setError("No variables were found.");

        return 0;
    }


    return 1;
}

/* ============================================================
   MATCH SINGLE CHARACTER
   ============================================================ */

int matchChar(char character)
{
    skipSpaces();

    if (expression[position] == character)
    {
        position++;

        return 1;
    }

    return 0;
}

/* ============================================================
   MATCH SYMBOLIC OPERATOR
   ============================================================ */

int matchString(const char *text)
{
    int length;

    skipSpaces();

    length = (int)strlen(text);

    if (strncmp(expression + position, text, length) == 0)
    {
        position += length;

        return 1;
    }


    return 0;
}

/* ============================================================
   MATCH WORD OPERATOR
   ============================================================ */

int matchKeyword(const char *keyword)
{
    int i;

    int length;

    skipSpaces();

    length = (int)strlen(keyword);

    for (i = 0; i < length; i++)
    {
        if (toupper((unsigned char)expression[position + i]) != keyword[i])
        {
            return 0;
        }
    }

    /*
       Prevent matching part of a larger word.

       Example:
       "OR" should not accidentally match
       inside another alphabetic word.
    */

    if (isalpha((unsigned char)expression[position + length]))
    {
        return 0;
    }

    position += length;

    return 1;
}

/* ============================================================
   FUNCTION DECLARATIONS FOR THE PARSER
   ============================================================ */

int parseExpression(void);

int parseBiconditional(void);

int parseImplication(void);

int parseOR(void);

int parseAND(void);

int parseNOT(void);

int parsePrimary(void);


/* ============================================================
   EXPRESSION
   ============================================================ */

int parseExpression(void)
{
    return parseBiconditional();
}

/* ============================================================
   BICONDITIONAL
   P <-> Q
   Precedence: Lowest
   ============================================================ */

int parseBiconditional(void)
{
    int left;

    left = parseImplication();

    while (!parseError && matchString("<->"))
    {
        int right;

        right = parseImplication();

        /*
           Biconditional is true when both
           sides have the same truth value.
        */

        left = (left == right);
    }

    return left;
}

/* ============================================================
   IMPLICATION
   P -> Q
   Equivalent to:
   NOT P OR Q

   Implication is right-associative.
   P -> Q -> R becomes:
   P -> (Q -> R)
   ============================================================ */

int parseImplication(void)
{
    int left;

    left = parseOR();

    if (!parseError && matchString("->"))
    {
        int right;

        right = parseImplication();

        /*
           P -> Q is equivalent to NOT P OR Q
        */

        return (!left || right);
    }

    return left;
}

/* ============================================================
   OR Supports: OR, |, ||
   ============================================================ */

int parseOR(void)
{
    int left;

    left = parseAND();

    while (!parseError)
    {
        if (matchString("||") || matchChar('|') || matchKeyword("OR"))
        {
            int right;

            right = parseAND();

            left = (left || right);
        }
        else
        {
            break;
        }
    }

    return left;
}

/* ============================================================
   AND Supports: AND, &, &&
   ============================================================ */

int parseAND(void)
{
    int left;

    left = parseNOT();

    while (!parseError)
    {
        if (matchString("&&") || matchChar('&') || matchKeyword("AND"))
        {
            int right;

            right = parseNOT();

            left = (left && right);
        }
        else
        {
            break;
        }
    }

    return left;
}


/* ============================================================
   NOT Supports: NOT, !
   ============================================================ */

int parseNOT(void)
{
    if (matchChar('!') || matchKeyword("NOT"))
    {
        return !parseNOT();
    }

    return parsePrimary();
}

/* ============================================================
   PRIMARY

   A primary can be:
    1. A variable
      P
      Q
      R

   OR

    2. A parenthesized expression
    (P AND Q)
   ============================================================ */

int parsePrimary(void)
{
    char variable;

    skipSpaces();

    /*
       Parenthesized expression
    */

    if (matchChar('('))
    {
        int result;

        result = parseExpression();

        if (!parseError && !matchChar(')'))
        {
            setError("Missing closing parenthesis.");
        }

        return result;
    }

    skipSpaces();

    variable = expression[position];

    /*
       Single uppercase variable
    */

    if (isupper((unsigned char)variable))
    {
        position++;

        return values[variable - 'A'];
    }

    /*
       Error conditions
    */

    if (expression[position] == '\0')
    {
        setError("Unexpected end of expression.");
    }
    else
    {
        setError("Expected a variable, NOT, or an opening parenthesis.");
    }

    return 0;
}

/* ============================================================
   EVALUATE THE COMPLETE EXPRESSION
   ============================================================ */

int evaluateExpression(void)
{
    int result;

    /*
       Every new truth-table row must start parsing from the beginning.
    */

    position = 0;

    parseError = 0;

    errorMessage[0] = '\0';

    result = parseExpression();

    skipSpaces();

    /*
       If there is still text remaining, the expression is invalid.
    */

    if (!parseError && expression[position] != '\0')
    {
        setError("Unexpected text after a complete expression.");
    }

    return result;
}

/* ============================================================
   PRINT SEPARATOR
   ============================================================ */

void printSeparator(int columns)
{
    int i;

    for (i = 0; i < columns; i++)
    {
        printf("--------");
    }

    printf("\n");
}

/* ============================================================
   PRINT VARIABLES
   ============================================================ */

void printVariables(void)
{
    int i;

    for (i = 0; i < variableCount; i++)
    {
        printf("%c\t", variables[i]);
    }
}

/* ============================================================
   MAIN PROGRAM
   ============================================================ */

int main(void)
{
    unsigned long long rows;

    unsigned long long row;

    int i;

    int bitPosition;

    int result;

    /*
       Used to classify the expression.
    */

    int allTrue = 1;

    int allFalse = 1;

    printf("============================================================\n");

    printf("        PROPOSITIONAL LOGIC TRUTH TABLE GENERATOR\n");

    printf("============================================================\n");


    printf("\nSUPPORTED OPERATORS:\n\n");

    printf("NOT          !\n");

    printf("AND          &       &&\n");

    printf("OR           |       ||\n");

    printf("IMPLICATION   ->\n");

    printf("BICONDITIONAL <->\n");

    printf("\nVARIABLE RULE:\n");

    printf("Variables must be single uppercase letters.\n");

    printf("Examples: P, Q, R, A, X, Y, Z\n");

    printf("\nEXAMPLE EXPRESSIONS:\n\n");

    printf("(P AND Q) -> R\n");

    printf("NOT(P OR Q) AND R\n");

    printf("((P AND Q) OR NOT R) -> Y\n");

    printf("(P <-> Q) AND (R -> S)\n");


    printf("\nEnter a logical expression:\n> ");

    /*
       Read complete expression including spaces.
    */

    if (fgets(expression, sizeof(expression), stdin) == NULL)
    {
        printf("\nInput error.\n");

        return 1;
    }


    /*
       Remove newline added by fgets.
    */
    expression[strcspn(expression,"\n")] = '\0';


    /* ========================================================
       DETECT VARIABLES
       ======================================================== */

    if (!detectVariables())
    {
        printf("\nERROR: %s\n", errorMessage);

        return 1;
    }


    /*
       Number of possible combinations.

       For n variables: rows = 2^n
    */

    rows = 1ULL << variableCount;


    /* ========================================================
       DISPLAY DETECTED VARIABLES
       ======================================================== */

    printf("\nDetected variables (%d): ", variableCount);

    for (i = 0; i < variableCount; i++)
    {
        printf("%c ", variables[i]);
    }

    printf("\nNumber of truth-table rows: %llu\n\n", rows);

    /* ========================================================
       PRINT TABLE HEADER
       ======================================================== */

    printVariables();

    printf("| RESULT\n");

    printSeparator(variableCount + 1);

    /* ========================================================
       GENERATE EVERY TRUTH TABLE ROW
       ======================================================== */

    for (row = 0; row < rows; row++)
    {
        /*
           Assign a binary value to every variable.

           Example for 3 variables:
           000
           001
           010
           011
           100
           101
           110
           111
        */

        for ( i = 0; i < variableCount; i++)
        {
            bitPosition = variableCount - 1 - i;

            values[variables[i] - 'A'] = ( row >> bitPosition ) & 1ULL;

            printf("%d\t", values[variables[i] - 'A']);
        }


        /* ====================================================
           EVALUATE THE ENTIRE EXPRESSION
           ==================================================== */

        result = evaluateExpression();

        /*
           Stop if expression syntax is invalid.
        */

        if (parseError)
        {
            printf("\nERROR WHILE EVALUATING:\n%s\n", errorMessage);

            return 1;
        }

        printf("| %d\n", result);

        /* ====================================================
           CLASSIFICATION CHECK
           ==================================================== */

        if (result)
        {
            allFalse = 0;
        }
        else
        {
            allTrue = 0;
        }
    }


    /* ========================================================
       FINAL CLASSIFICATION
       ======================================================== */

    printf(
        "\n============================================================\n"
    );

    printf("CLASSIFICATION: ");

    if (allTrue)
    {
        printf("TAUTOLOGY\n");
    }

    else if (allFalse)
    {
        printf("CONTRADICTION\n");
    }

    else
    {
        printf("CONTINGENCY\n");
    }


    printf(
        "============================================================\n"
    );

    return 0;
}
