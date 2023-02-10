#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "compiler.h"
#include "cpu.h"
#include "shared.h"
#include "linkedList.h"


// 0x300 is the default starting address
// this can be manually set by putting the desired start address
// in the first line of a program
#define START_ADDR 0x300
int startFound = 0;
int addr = START_ADDR;
Node *head = NULL;

#ifdef _WIN32
#else
char *strlwr(char *str)
{
    unsigned char *p = (unsigned char *)str;

    while (*p) {
        *p = tolower((unsigned char)*p);
        p++;
    }

    return str;
}
#endif

char prevChar(const char *line, int pos)
{
    if (pos > 0)
        return line[pos - 1];

    return '\0';
}

char nextChar(const char *line, int pos)
{
    if (pos < strlen(line))
        return line[pos + 1];
    
    return '\0';
}

int isWhiteSpaceChar(const char c)
{
    return (c == ' ' || c == '\n' || c == '\t');
}

int getRegisterNumber(const char *line, int pos)
{
    int len = 0;
    int initPos = pos;
    while (!isWhiteSpaceChar(line[pos]) && line[pos] != ',')
    {
        pos++;
        len++;
    }

    char *regNum = malloc(sizeof(char)*len + 1);
    strncpy(regNum, line + (initPos+1), len);
    regNum[len-1] = '\0';
    return atoi(regNum);
}

int getNumberVal(const char *line, int pos)
{
    int len = 0;
    int initPos = pos;
    int base = 10;
    while (!isWhiteSpaceChar(line[pos]))
    {
        pos++;
        len++;
    }
 
    if (line[initPos] == '0' && nextChar(line, initPos) == 'x')
    {
        len -= 1;
        initPos += 2;
        base = 16;
    }

    char *num = malloc(sizeof(char)*len + 1);
    strncpy(num, line + (initPos), len);
    num[len] = '\0';
    return (int)strtol(num, NULL, base);
}

char* getSubstr(const char *line, int start, int end)
{
    if(end<start)
        return NULL;
    char* substr = malloc(sizeof(char)*((end-start) +1));
    int i = 0;
    for (int x = start; x < end; x++)
    {
        substr[i] = line[x];
        i++;
    }
    substr[i] = '\0';
    return substr;
}

char *removeComments(char *line)
{
    int pos = 0;
    int len = 0;
    char *result;
    while (line[pos] != '\n' && line[pos] != '\0')
    {
        if (line[pos] == '/' && prevChar(line, pos) == '/')
        {
            pos -= 1;
            //printSubstr(line, 0, pos);
            result = malloc(sizeof(char)*len);
            strncpy(result, line, len);
            result[(pos)] = '\n';
            result[(pos + 1)] = '\0';
            return result;
        }
        len++;
        pos++;
    }
    return line;
}

void replaceTabs(char *line)
{
    int pos = 0;
    while (line[pos] != '\0')
    {
        if (line[pos] == '\t')
            line[pos] = ' ';
        pos++;
    }
}

char *removeExcessWhiteSpace(char* line)
{
    int pos = 0;
    int resultIndex = 0;
    int charFound = 0;
    while (line[pos] != '\n' && line[pos] != '\0')
    {
        if (!isWhiteSpaceChar(line[pos]) && !charFound)
            charFound = 1;

        if (!(line[pos] == ' ' && prevChar(line, pos) == ' ') && charFound)
        {
            line[resultIndex] = line[pos];
            resultIndex++;
        }

        pos++;
    }
    // Ensure the line ends in a newline followed by
    // a null terminator.
    if(prevChar(line, resultIndex)==' ')
    {
        line[resultIndex-1] = '\n';
        line[resultIndex] = '\0';
    }
    else
    {
        line[resultIndex] = '\n';
        line[resultIndex+1] = '\0';
    }


    return line;
}

void removeCarrigeReturn(char* line)
{
    int pos = 0;
    while (line[pos] != '\0')
    {
        if (line[pos] == '\r')
            line[pos] = '\n';
        pos++;
    }
}

int getBitShift(int numElementsSet)
{
    switch (numElementsSet)
    {
    case 1:
        return 16;
    case 2:
        return 8;
    case 3:
        return 0;
    default:
        return 24;
    }
}

int getRegister(char *line, int pos, int elementsSet, int *hexInst)
{
    //printf("%s, nextChar = %c\n", hexToOpcode(*hexInst >> 24), nextChar(line, pos));
    if (*hexInst >> 24 == MOV && elementsSet == 2)
        elementsSet++;

    if (line[pos] == 'r')
    {
        *hexInst |= getRegisterNumber(line, pos) << getBitShift(elementsSet);
        return 1;
    }
    else if (line[pos] == 's' && nextChar(line, pos) == 'r')
    {
        *hexInst |= SR << getBitShift(elementsSet); 
        //elementsSet++;
        return 1;
    }
    else if (line[pos] == 'l' && nextChar(line, pos) == 'r')
    {
        *hexInst |= LR << getBitShift(elementsSet);
        //elementsSet++;
        return 1;
    }
    else if (line[pos] == 'p' && nextChar(line, pos) == 'c')
    {
        *hexInst |= PC << getBitShift(elementsSet);
        //elementsSet++;
        return 1;
    }
    return 0;
}

void storeFunction(char *funcName)
{
    char *func = malloc(strlen(funcName) - 1);
    strncpy(func, funcName, strlen(funcName) - 1);
    func[strlen(funcName) - 2] = '\0';
    if (strcmp(func, "_start") == 0)
        startFound = 1;

    head = addNode(func, addr, head);
}

int functionToAddress(char *line, int pos)
{
    int start = pos;
    while (line[pos] != '\n' && line[pos] != '\0')
        pos++;
    char *funcName = malloc(sizeof(char)*(pos - start) + 1);
    strncpy(funcName, line + start, (pos - start));
    funcName[pos - start] = '\0';
    int functionAddress = findNode(funcName, head);
    if (functionAddress == -1)
    {
        printf("The function \'%s\' is not in the program", funcName);
        exit(EXIT_FAILURE);
    }
    return functionAddress;
}

int getMemStart(char *buffer)
{
    int pos = 0;
    int start = 0;

    if (buffer[pos] == '#')
    {
        pos++;
        start++;
    }
    char *temp = &buffer[pos];
    if (!isdigit((unsigned char)buffer[pos]) && strtol(temp,NULL, 16) == 0)
        return -1;

    while (buffer[pos] != '\n' && buffer[pos] != '\0')
        pos++;

    char *line = malloc(sizeof(char)*(pos-start) + 1);
    strncpy(line, buffer+start, (pos-start));
    line[pos] = '\0';
    if(verbose)
        printf("First line: %s\n", line);
    return (int)strtol(line, NULL, 16);
}

char *asmToHex(char *line)
{
    int prevPos = 0;
    int pos = 0;
    int opcodeVal = 0;
    int elementsSet = 0;
    int hexInst = 0;
    int isAddress = 0;

    while (line[pos] != '\0')
    {
        if (line[pos] == ' ' && prevChar(line, pos) != ',' && prevPos == 0)
        {
            size_t size = (pos - prevPos) + 1;
            char *opcode = malloc(sizeof(char)*(size));
            strncpy(opcode, line+prevPos, size);
            opcode[size-1] = '\0';
            opcodeVal = opcodeToHex(opcode);
            
            hexInst = opcodeVal << getBitShift(elementsSet);
            prevPos = pos+1;
            elementsSet++;
            //if (strcmp(opcode, "cmp") == 0)
            //    elementsSet++;
        }
        else if (line[pos] == ',')
        {
            if (getRegister(line, prevPos, elementsSet, &hexInst))
            {
                prevPos = pos + 2;
                elementsSet++;
                //if (opcodeVal == MOV)
                //    elementsSet++;
            }
        }
        else if (isWhiteSpaceChar(line[pos]) && prevChar(line, pos) != ',')
        {   
            if (getRegister(line, prevPos, elementsSet, &hexInst))
            {
                prevPos = pos + 1;
                elementsSet++;
            }
        }
        else if (line[pos] == '#')
        {
            pos++;
            hexInst |= getNumberVal(line, pos);
            if ((opcodeVal == MOV || opcodeVal == ADD || opcodeVal == SUB ||
                    opcodeVal == MUL || opcodeVal == DIV || opcodeVal == CMP) && !isAddress)
                hexInst |= 1 << 20;
        }
        else if (line[pos] == '=')
        {
            pos++;
            isAddress = 1;
            hexInst |= functionToAddress(line, pos);
        }
        else if (line[pos] == '[')
        {
            pos++;
            isAddress = 1;
            if (getRegister(line, pos, elementsSet, &hexInst))
            {
                if (opcodeVal == MOV)
                    hexInst |= 2 << 20;
            }
            elementsSet++;
        }
            
        pos++;
    }
    char *result = malloc(sizeof(char) * 11);
    sprintf(result, "0x%08x", hexInst);
    return result;
}

char *getOutFileName(char *filename)
{
    char *c = &filename[strlen(filename)];
    while (1)
    {
        if (*c-- == '.')
            break;
    }
    
    // Add three to re-include the '.', an additional character
    // for the 'o' so it can be a '.o' file, then, a final third
    // character for '\0'
    int len = (int)(strlen(filename) - strlen(c));
    len += 3;

    char *result = malloc(sizeof(char)*len);
    strncpy(result, filename, len);
    result[len - 1] = 'o';
    result[len] = '\0';

    return result;
}

char *parseDataArg(char *line)
{
    char *argument = malloc(sizeof(char) * 6);
    strncpy(argument, line, 6);
    argument[6] = '\0';

    return argument;
}

void handelArrayData(char *line, char *argument, FILE *fout)
{
    if(strcmp(argument, ".iarr ") == 0)
    {
        int startPos = 6;
        // Doesn't allow for elements on multiple lines
        // isn't exactly ideal but this is just as a proof of concept
        while(line[startPos] != '{' && line[startPos] != '\n')
            startPos++;
        startPos++;
        int pos = startPos;
        while(line[pos] != '}' && line[pos] != '\n')
        {
            int digitFound = 0;
            while(line[pos] != ',' && line[pos] != '}' && line[pos] != '\n')
            {
                if(isdigit((unsigned char)line[pos]) && !digitFound)
                {
                    startPos = pos;
                    digitFound = 1;
                }
                pos++;
            }

            char *num = getSubstr(line, startPos, pos);
            int base = 10;
            if(num[0]=='0' && nextChar(num, 0) == 'x')
                base = 16;
            int arrVal = (int)strtol(num, NULL, base);
            fprintf(fout, "0x%08x\n", arrVal);
            pos++;
        }
    }
}

char *getDataFromArg(char *line, char *argument)
{
    if (strcmp(argument, ".ascii") == 0)
    {
        char *str = line + 8;
        int len = (int)strlen(str) - 1;
        if (str[len] == '\n')
            len--;
        str[len] = '\0';
        return str;
    }
    else if (strcmp(argument, ".skip ") == 0)
        return line + 6;
    else if(strcmp(argument, ".iarr ") == 0)
    {
        int elements = 0;
        int startPos = 6;
        // Buggy, needs to be more fleshed out
        // this is just as a proof of concept
        while(line[startPos] != '{' && line[startPos] != '\n')
            startPos++;
        startPos++;
        int pos = startPos;
        while(line[pos] != '}' && line[pos] != '\n')
        {
            int digitFound = 0;
            while(line[pos] != ',' && line[pos] != '}' && line[pos] != '\n')
            {
                if(isdigit((unsigned char)line[pos]) && !digitFound)
                {
                    startPos = pos;
                    digitFound = 1;
                    elements++;
                }
                pos++;
            }
            pos++;
        }
        char *result = malloc(sizeof(int));
        snprintf(result, sizeof(int), "%d", elements);
        return result;
    }
    return "\0";
}

int handleDataSectionFunction(char *line)
{
    int addrAdvance = 0;

    char *arg = parseDataArg(line);
    char *data = getDataFromArg(line, arg);

    //printf("Arg: %s\n", arg);
    if (strcmp(arg, ".ascii") == 0)
    {
        //printf("Data len: %d", strlen(data));
        addrAdvance = (int)strlen(data);
    }
    else if (strcmp(arg, ".skip ") == 0)
    {
        addrAdvance = (int)strtol(data, NULL, 10); //* 4;
    }
    else if (strcmp(arg, ".iarr ") == 0)
    {
        addrAdvance = (int)strtol(data, NULL, 10) * 4;
    }

    return addrAdvance;
}

void storeFunctionNames(char *buffer)
{
    int lineStart = 0;
    int pos = 0;
    int dataBegin = 0;
    while (buffer[pos] != '\0')
    {
        int lineParsed = 0;
        char *line;
        while (buffer[pos] != '\n' && buffer[pos] != '\0')
            pos++;

        size_t lineLen = (pos - lineStart) + 1;
        line = malloc(sizeof(char)*lineLen);
        strncpy(line, buffer + lineStart, lineLen);
        line[lineLen] = '\0';
//        printf("%s", line);

        replaceTabs(line);
        removeCarrigeReturn(line);
        char *noComments = removeComments(line);
        char *noWhiteSpace = removeExcessWhiteSpace(noComments);
        if (strcmp(noWhiteSpace, "section .text\n") == 0)
        {
            dataBegin = 0;
            lineParsed = 1;
        }
        else if (strcmp(noWhiteSpace, "section .data\n") == 0)
        {
            dataBegin = 1;
            lineParsed = 1;
        }
        //free(noComments);
        if (strcmp(noWhiteSpace, "\n") != 0 && strcmp(noWhiteSpace, "\0") != 0)
        {
            if (noWhiteSpace[strlen(noWhiteSpace) - 2] == ':')
            {
                //addr -= 4;
                if(verbose)
                    printf("Function: %s", noWhiteSpace);
                storeFunction(noWhiteSpace);
                lineParsed = 1;
                addr += 4;
            }
            //free(noWhiteSpace);
            if (!lineParsed && dataBegin)
            {
                int addrAdvance = handleDataSectionFunction(noWhiteSpace);
                addr += addrAdvance;
            }
            else if (!lineParsed)
                addr += 4;

        }

        pos++;
        lineStart = pos;
    }
}

char handelEscapeChars(char *line, int pos)
{
    // Not inclusive yet
    switch(nextChar(line,pos))
    {
        case 'n':
            return '\n';
        case '0':
            return '\0';
        case 't':
            return '\t';
        case '\\':
            return '\\';
        case 'r':
            return '\r';
        default:
            return '\n';
    }
}

void handleDataSectionAssembly(char *line, FILE *fout)
{
    int hexInst = 0;

    char *arg = parseDataArg(line);
    char *data = getDataFromArg(line, arg);

    //printf("Arg: %s\n", arg);
    if (strcmp(arg, ".ascii") == 0)
    {
        //for (int x = 0; x < strlen(data); x++)
        //    printf("%c", data[x]);
        //printf("\n");
        int pos = 0;
        int count = 0;
        int skipNext = 0;
        while (data[pos] != '\0')
        {
            if(!skipNext)
            {
                if (count % 4 == 0 && count != 0)
                {
                    fprintf(fout, "0x%08x\n", hexInst);
                    count = 0;
                    hexInst = 0;
                }
                if(data[pos]=='\\' && prevChar(data, pos)!='\\')
                {
                    data[pos] = handelEscapeChars(data, pos);
                    skipNext = 1;
                }
                else
                    skipNext = 0;

                hexInst |= data[pos] << getBitShift(count);
                count++;
            }
            pos++;
        }
        if (hexInst != 0)
            fprintf(fout, "0x%08x\n", hexInst);

        addr += strlen(data);
    }
    else if (strcmp(arg, ".skip ") == 0)
    {
        int emptyBytes = (int)strtol(data, NULL, 10); //*4;
        addr += emptyBytes;
        for (int x = emptyBytes; x > 0;x-=4)
            fprintf(fout, "0x%08x\n", hexInst);
    }
    else if (strcmp(arg, ".iarr ") ==0)
    {
        handelArrayData(line, arg, fout);
        addr+= (int)strtol(data, NULL, 10) * 4;
    }
    else
    {
        fprintf(fout, "0x%08x\n", hexInst);
        addr += 4;
    }


    //char *result = malloc(sizeof(char) * 11);
    //sprintf(result, "0x%08x", hexInst);

    //return result;
}

void assemblyToHexData(char *buffer, FILE *fout)
{
    int lineStart = 0;
    int pos = 0;
    int dataBegins = 0;
    while (buffer[pos] != '\0')
    {
        int lineParsed = 0;
        char *line;
        while (buffer[pos] != '\n' && buffer[pos] != '\0')
            pos++;

        size_t lineLen = (pos - lineStart) + 1;
        line = malloc(lineLen);
        strncpy(line, buffer + lineStart, lineLen);
        line[lineLen] = '\0';
        //printf("%s", line);

        replaceTabs(line);
        removeCarrigeReturn(line);
        char *noComments = removeComments(line);
        char *noWhiteSpace = removeExcessWhiteSpace(noComments);

        if (strcmp(noWhiteSpace, "section .data\n") == 0)
        {
            dataBegins = 1;
            lineParsed = 1;
        }

        //free(noComments);
        if (strcmp(noWhiteSpace, "\n") != 0 && strcmp(noWhiteSpace, "\0") != 0 && (!lineParsed && dataBegins))
        {

            //char *hexInst = handleDataSectionAssembly(noWhiteSpace);
            //// Add new instruction to output file
            //fprintf(fout, "%s\n", hexInst);
            handleDataSectionAssembly(noWhiteSpace, fout);
            //printf("Instruction: %s, line: %s", hexInst, noWhiteSpace);

            //free(noWhiteSpace);
        }

        pos++;
        lineStart = pos;
    }
}

void assemblyToHexInstructions(char *buffer, FILE *fout)
{
    int lineStart = 0;
    int pos = 0;
    int codeBegins = 0;
    while (buffer[pos] != '\0')
    {
        int lineParsed = 0;
        char *line;
        while (buffer[pos] != '\n' && buffer[pos] != '\0')
            pos++;

        size_t lineLen = (pos - lineStart) + 1;
        line = malloc(lineLen);
        strncpy(line, buffer + lineStart, lineLen);
        line[lineLen] = '\0';
        //printf("%s", line);

        replaceTabs(line);
        removeCarrigeReturn(line);
        char *noComments = removeComments(line);
        char *noWhiteSpace = removeExcessWhiteSpace(noComments);
        if (strcmp(noWhiteSpace, "section .text\n") == 0)
        {
            codeBegins = 1;
            lineParsed = 1;
        }
        else if (strcmp(noWhiteSpace, "section .data\n") == 0)
            break;

        //free(noComments);
        if (strcmp(noWhiteSpace, "\n") != 0 && strcmp(noWhiteSpace, "\0") != 0 && (!lineParsed && codeBegins))
        {
            char *hexInst = asmToHex(noWhiteSpace);
            // Add new instruction to output file
            fprintf(fout, "%s\n", hexInst);
            if (verbose)
                printf("Instruction: %s, Assembly: %s", hexInst, noWhiteSpace);

            //free(noWhiteSpace);
            addr += 4;
        }

        pos++;
        lineStart = pos;
    }
}

programInfo *compileProg(char *programFile)
{
    programInfo *progInfo = malloc(sizeof(programInfo));
    progInfo->memStart = START_ADDR;
    progInfo->progStart = START_ADDR;
    FILE *fin = fopen(programFile, "r");
    progInfo->progName = getOutFileName(programFile);
    FILE *fout = fopen(progInfo->progName, "w");
    char *buffer;

    if (fin != NULL && fout != NULL)
    {
        fseek(fin, 0, SEEK_END);
        long size = ftell(fin);
        fseek(fin, 0, SEEK_SET);

        buffer = malloc(size+1);
        size_t readBytes = fread(buffer, 1, size, fin);
        fclose(fin);
        buffer[readBytes] = '\0';

        buffer = strlwr(buffer);

        int memoryStart = getMemStart(buffer);
        if(verbose)
            printf("Mem start: %d\n", memoryStart);
        if (memoryStart == -1)
            memoryStart = START_ADDR;

        fprintf(fout, "0x%08x\n", memoryStart);
        progInfo->memStart = memoryStart;
        progInfo->progStart = memoryStart;
        addr = memoryStart;
        storeFunctionNames(buffer);

        if (startFound)
            progInfo->progStart = findNode("_start", head);

        addr = memoryStart;
        assemblyToHexInstructions(buffer, fout);
        assemblyToHexData(buffer, fout);

        fclose(fout);
    }
    else
    {
        perror(programFile);
        exit(EXIT_FAILURE);
    }

    return progInfo;
}
