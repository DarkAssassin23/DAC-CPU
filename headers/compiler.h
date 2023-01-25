#pragma once

typedef struct programInfo
{
    int memStart;
    int progStart;
    char *progName;
} programInfo;

char* getSubstr(const char *line, int start, int end);
programInfo * compileProg(char *programFile);