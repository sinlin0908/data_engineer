
#if !defined(_DS_H)
#define _DS_H

#include <wchar.h>
#include <stdlib.h>

typedef char *CharPtr;
typedef wchar_t *WcharPtr;

typedef struct data
{
    WcharPtr title;
    WcharPtr url;
    WcharPtr *content;
    size_t sentence_count;
} Data;

Data *createOneNewsBuffer();

WcharPtr createField(WcharPtr line);

int cmpChar(const void *a, const void *b);
void sort(wchar_t **content, size_t cnt);

#endif // _DS_H
