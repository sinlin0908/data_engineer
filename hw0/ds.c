#include "ds.h"

Data *createOneNewsBuffer()
{
    Data *ptr = (Data *)malloc(sizeof(Data));
    ptr->title = NULL;
    ptr->url = NULL;
    ptr->content = NULL;
    ptr->sentence_count = 0;

    return ptr;
}

WcharPtr createField(WcharPtr line)
{
    WcharPtr ptr = (WcharPtr)malloc(sizeof(wchar_t) * wcslen(line) + 3);
    wcscpy(ptr, line);

    return ptr;
}

int cmpChar(const void *a, const void *b)
{
    return wcscmp(*(wchar_t **)a, *(wchar_t **)b);
}

void sort(wchar_t **input, size_t cnt)
{
    qsort((void *)input, cnt, sizeof(wchar_t *), cmpChar);
}