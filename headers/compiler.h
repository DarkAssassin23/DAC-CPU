#pragma once

typedef struct programInfo
{
    int memStart;
    int progStart;
    char *progName;
} programInfo;

programInfo * compileProg(char *programFile);