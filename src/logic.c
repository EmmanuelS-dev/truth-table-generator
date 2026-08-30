#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_EXPRESSION 1000
#define MAX_VARIABLES 20

/* ============================================================
   GLOBAL ENGINE STATE
   ============================================================ */

static char expression[MAX_EXPRESSION];
static char variables[MAX_VARIABLES];
static int variableCount = 0;
static int values[26];
static int position = 0;
static int parseError = 0;
static char errorMessage[256];

/* ============================================================
   ERROR HANDLING
   ============================================================ */

static void setError(const char *message)
{
    if (!parseError)
    {
        parseError = 1;
        strncpy(errorMessage, message, sizeof(errorMessage) - 1);
        errorMessage[sizeof(errorMessage) - 1] = '\0';
    }
}

/* ============================================================
   SPACE HANDLING & HELPER FUNCTIONS
   ============================================================ */

static void skipSpaces(void)
{
    while (isspace((unsigned char)expression[position]))
    {
        position++;
    }
}

static int wordEqualsIgnoreCase(const char *word, int length, const char *keyword)
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

static void addVariable(char variable)
{
    int i;
    for (i = 0; i < variableCount; i++)
    {
        if (variables[i] == variable)
        {
            return;
        }
    }

    if (variableCount >= MAX_VARIABLES)
    {
        setError("Too many distinct variables (maximum allowed is 20).");
        return;
    }

    variables[variableCount] = variable;
    variableCount++;
}

/* ============================================================
   DETECT VARIABLES
   ============================================================ */

static int detectVariables(void)
{
    int i = 0;
    variableCount = 0;
    parseError = 0;
    errorMessage[0] = '\0';

    while (expression[i] != '\0')
    {
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

            /* Ignore logical operator keywords */
            if (wordEqualsIgnoreCase(word, length, "AND") ||
                wordEqualsIgnoreCase(word, length, "OR") ||
                wordEqualsIgnoreCase(word, length, "NOT"))
            {
                continue;
            }

            /* Variable must be a single uppercase letter */
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
                setError("Variables must be single uppercase letters (e.g. P, Q, R). Use AND, OR, and NOT for operators.");
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
        setError("No propositional variables found in expression.");
        return 0;
    }

    return 1;
}

/* ============================================================
   TOKEN MATCHING FUNCTIONS
   ============================================================ */

static int matchChar(char character)
{
    skipSpaces();
    if (expression[position] == character)
    {
        position++;
        return 1;
    }
    return 0;
}

static int matchString(const char *text)
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

static int matchKeyword(const char *keyword)
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

    /* Prevent matching prefix of a larger word */
    if (isalpha((unsigned char)expression[position + length]))
    {
        return 0;
    }

    position += length;
    return 1;
}

/* ============================================================
   RECURSIVE DESCENT PARSER DECLARATIONS & IMPLEMENTATION
   ============================================================ */

static int parseExpression(void);
static int parseBiconditional(void);
static int parseImplication(void);
static int parseOR(void);
static int parseAND(void);
static int parseNOT(void);
static int parsePrimary(void);

static int parseExpression(void)
{
    return parseBiconditional();
}

static int parseBiconditional(void)
{
    int left = parseImplication();

    while (!parseError && (matchString("<->") || matchString("<=>")))
    {
        int right = parseImplication();
        left = (left == right);
    }
    return left;
}

static int parseImplication(void)
{
    int left = parseOR();

    if (!parseError && (matchString("->") || matchString("=>")))
    {
        int right = parseImplication();
        return (!left || right);
    }
    return left;
}

static int parseOR(void)
{
    int left = parseAND();

    while (!parseError)
    {
        if (matchString("||") || matchChar('|') || matchKeyword("OR"))
        {
            int right = parseAND();
            left = (left || right);
        }
        else
        {
            break;
        }
    }
    return left;
}

static int parseAND(void)
{
    int left = parseNOT();

    while (!parseError)
    {
        if (matchString("&&") || matchChar('&') || matchKeyword("AND"))
        {
            int right = parseNOT();
            left = (left && right);
        }
        else
        {
            break;
        }
    }
    return left;
}

static int parseNOT(void)
{
    if (matchChar('!') || matchChar('~') || matchKeyword("NOT"))
    {
        return !parseNOT();
    }
    return parsePrimary();
}

static int parsePrimary(void)
{
    char variable;

    skipSpaces();

    if (matchChar('('))
    {
        int result = parseExpression();
        if (!parseError && !matchChar(')'))
        {
            setError("Missing closing parenthesis ')'.");
        }
        return result;
    }

    skipSpaces();
    variable = expression[position];

    if (isupper((unsigned char)variable))
    {
        position++;
        return values[variable - 'A'];
    }

    if (expression[position] == '\0')
    {
        setError("Unexpected end of expression.");
    }
    else
    {
        setError("Expected a variable, NOT, or an opening parenthesis '('");
    }

    return 0;
}

static int evaluateExpression(void)
{
    int result;
    position = 0;
    parseError = 0;
    errorMessage[0] = '\0';

    result = parseExpression();
    skipSpaces();

    if (!parseError && expression[position] != '\0')
    {
        setError("Unexpected extra character(s) after complete expression.");
    }

    return result;
}

/* ============================================================
   JSON STRING BUILDER HELPER
   ============================================================ */

static void jsonEscapeAppend(char **buffer, size_t *capacity, size_t *length, const char *str)
{
    size_t i;
    for (i = 0; str[i] != '\0'; i++)
    {
        char ch = str[i];
        if (ch == '"' || ch == '\\')
        {
            if (*length + 2 >= *capacity)
            {
                *capacity *= 2;
                *buffer = (char *)realloc(*buffer, *capacity);
            }
            (*buffer)[(*length)++] = '\\';
            (*buffer)[(*length)++] = ch;
        }
        else
        {
            if (*length + 1 >= *capacity)
            {
                *capacity *= 2;
                *buffer = (char *)realloc(*buffer, *capacity);
            }
            (*buffer)[(*length)++] = ch;
        }
    }
    (*buffer)[*length] = '\0';
}

/* ============================================================
   EXPORTED WEBASSEMBLY API
   ============================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/*
   Frees dynamic memory allocated by evaluate_logic.
*/
void free_result(char *ptr)
{
    if (ptr)
    {
        free(ptr);
    }
}

/*
   Evaluates a logical expression and returns a dynamically allocated
   JSON string containing variables, truth table rows, and classification.
*/
char *evaluate_logic(const char *input_expr)
{
    size_t capacity = 4096;
    size_t length = 0;
    char *json = (char *)malloc(capacity);
    if (!json)
    {
        return NULL;
    }
    json[0] = '\0';

    /* Reset global engine state */
    variableCount = 0;
    position = 0;
    parseError = 0;
    errorMessage[0] = '\0';
    memset(values, 0, sizeof(values));
    memset(variables, 0, sizeof(variables));

    if (input_expr == NULL || strlen(input_expr) == 0)
    {
        snprintf(json, capacity, "{\"success\":false,\"error\":\"Expression cannot be empty.\"}");
        return json;
    }

    if (strlen(input_expr) >= MAX_EXPRESSION)
    {
        snprintf(json, capacity, "{\"success\":false,\"error\":\"Expression is too long (maximum 1000 characters).\"}");
        return json;
    }

    strncpy(expression, input_expr, MAX_EXPRESSION - 1);
    expression[MAX_EXPRESSION - 1] = '\0';

    if (!detectVariables())
    {
        size_t cap = 512;
        size_t len = 0;
        char *err_json = (char *)malloc(cap);
        snprintf(err_json, cap, "{\"success\":false,\"error\":\"");
        len = strlen(err_json);
        jsonEscapeAppend(&err_json, &cap, &len, errorMessage);
        snprintf(err_json + len, cap - len, "\"}");
        free(json);
        return err_json;
    }

    unsigned long long rowsCount = 1ULL << variableCount;
    int allTrue = 1;
    int allFalse = 1;

    /* Build JSON Header */
    int written = snprintf(json + length, capacity - length, "{\"success\":true,\"varCount\":%d,\"rowCount\":%llu,\"variables\":[", variableCount, rowsCount);
    length += written;

    int i;
    for (i = 0; i < variableCount; i++)
    {
        written = snprintf(json + length, capacity - length, "\"%c\"%s", variables[i], (i == variableCount - 1) ? "" : ",");
        length += written;
    }

    written = snprintf(json + length, capacity - length, "],\"rows\":[");
    length += written;

    unsigned long long row;
    for (row = 0; row < rowsCount; row++)
    {
        /* Set binary truth values for variables */
        for (i = 0; i < variableCount; i++)
        {
            int bitPosition = variableCount - 1 - i;
            values[variables[i] - 'A'] = (row >> bitPosition) & 1ULL;
        }

        int result = evaluateExpression();
        if (parseError)
        {
            free(json);
            size_t cap = 512;
            size_t len = 0;
            char *err_json = (char *)malloc(cap);
            snprintf(err_json, cap, "{\"success\":false,\"error\":\"");
            len = strlen(err_json);
            jsonEscapeAppend(&err_json, &cap, &len, errorMessage);
            snprintf(err_json + len, cap - len, "\"}");
            return err_json;
        }

        if (result)
        {
            allFalse = 0;
        }
        else
        {
            allTrue = 0;
        }

        /* Ensure JSON buffer has space for row */
        if (length + 256 + (variableCount * 4) >= capacity)
        {
            capacity *= 2;
            json = (char *)realloc(json, capacity);
        }

        written = snprintf(json + length, capacity - length, "{\"values\":[");
        length += written;

        for (i = 0; i < variableCount; i++)
        {
            written = snprintf(json + length, capacity - length, "%d%s", values[variables[i] - 'A'], (i == variableCount - 1) ? "" : ",");
            length += written;
        }

        written = snprintf(json + length, capacity - length, "],\"result\":%d}%s", result, (row == rowsCount - 1) ? "" : ",");
        length += written;
    }

    const char *classification = "CONTINGENCY";
    if (allTrue)
    {
        classification = "TAUTOLOGY";
    }
    else if (allFalse)
    {
        classification = "CONTRADICTION";
    }

    if (length + 128 >= capacity)
    {
        capacity += 128;
        json = (char *)realloc(json, capacity);
    }

    snprintf(json + length, capacity - length, "],\"classification\":\"%s\"}", classification);
    return json;
}

#ifdef __cplusplus
}
#endif

#ifndef EMSCRIPTEN
int main(void)
{
    printf("Logical Expression Engine compiled in standalone mode.\n");
    char *res = evaluate_logic("(P AND Q) -> R");
    if (res)
    {
        printf("Result JSON:\n%s\n", res);
        free_result(res);
    }
    return 0;
}
#endif
