#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <locale.h>
#include "ds.h"

typedef Data *DataPtr;

const size_t MAX_FILES = 6;
const size_t MAX_LINE = 100;
const size_t MIN_CHAR_LEN = 5;
const size_t MAX_BUFFER_SIZE = 4096;

const unsigned long MIN_CHINESE_CODE = 0x4E00;
const unsigned long MAX_CHINESE_CODE = 0x9FD5;

const int TRUE = 1;
const int FALSE = 0;

const WcharPtr tokens = L"。？！\n?!";

const CharPtr dir_name = "./files";
const CharPtr output_file_name = "./data.txt";

size_t total_sentence_cnt = 0;

void getTargetFileNames(CharPtr files[]);
/*
* @ Get file names we want to parse.
*/

void parseFiles(CharPtr files[]);
/*
* @ parse file to which we want.
*/

size_t splitSentence(WcharPtr **sentences, WcharPtr content);

int isChinese(const wchar_t first_c);
/*
* @ check whether the character is Chinese.
*/

int countChineseChar(WcharPtr test);
/*
* @ count chinese a sentence.
*/

void outputOneNews(FILE *output_file, DataPtr data);

void sortOutputFile();

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL, "");

    CharPtr files[MAX_FILES];

    getTargetFileNames(files);

    parseFiles(files);

    printf("total sentences: %ld\n", total_sentence_cnt);

    printf("Start Sorting\n");
    sortOutputFile();

    return 0;
}

void getTargetFileNames(CharPtr files[])
{
    struct dirent *dir_entry;

    DIR *dir = opendir(dir_name);

    if (dir == NULL)
    {
        printf("Couldn't find dir!\n");
        exit(1);
    }

    int file_count = 0;
    while ((dir_entry = readdir(dir)) != NULL)
    {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
        {
            continue;
        }
        else
        {
            int len = strlen(dir_entry->d_name);
            files[file_count] = (CharPtr)malloc(sizeof(char) * len + 3);
            if (!files[file_count])
            {
                printf("Malloc Error\n");
            }
            strcpy(files[file_count], dir_entry->d_name);
            file_count++;
        }
    }
    closedir(dir);
}

void parseFiles(CharPtr files[])
{
    FILE *fp = NULL;
    FILE *output_file = NULL;

    WcharPtr buffer = NULL;

    /*---- Open output file ----*/
    output_file = fopen(output_file_name, "w");
    if (!output_file)
    {
        printf("Open output file Error!!\n");
        exit(1);
    }

    /*---- Create read buffer ----*/
    buffer = (WcharPtr)malloc(sizeof(wchar_t) * MAX_BUFFER_SIZE);
    if (!buffer)
    {
        printf("Buffer malloc error!!\n");
        exit(1);
    }

    /*---- Open input file ----*/
    for (size_t i = 0; i < MAX_FILES; i++)
    {
        size_t len = strlen(files[i]) + strlen(dir_name) + 3;
        CharPtr file_path = (CharPtr)malloc(sizeof(char) * len);

        strcpy(file_path, dir_name);
        strcat(file_path, "/");
        strcat(file_path, files[i]);

        printf("f: %s\n", file_path);

        fp = fopen(file_path, "r");
        if (!fp)
        {
            printf("Open Error!!\n");
            exit(1);
        }

        while (fgetws(buffer, MAX_BUFFER_SIZE, fp) != NULL)
        {
            if (wcsncmp(buffer, L"@GAISRec:", 9) != 0) // find an article head
            {
                continue;
            }
            else
            {
                /*---- Create data struct ----*/
                DataPtr one_news = createOneNewsBuffer();
                /*---- Get URL ----*/
                fgetws(buffer, MAX_BUFFER_SIZE, fp);
                buffer[wcslen(buffer) - 1] = L'\0';

                one_news->url = createField(buffer);
                /*---- Get Title ----*/
                fgetws(buffer, MAX_BUFFER_SIZE, fp);
                buffer[wcslen(buffer) - 1] = L'\0';

                one_news->title = createField(buffer);
                /*---- Get Content ----*/
                fgetws(buffer, MAX_BUFFER_SIZE, fp); //@B:
                fgetws(buffer, MAX_BUFFER_SIZE, fp); // Content
                one_news->sentence_count = splitSentence(&one_news->content, buffer);
                total_sentence_cnt += one_news->sentence_count;

                outputOneNews(output_file, one_news);

                free(one_news);
                one_news = NULL;
            }
        }

        fclose(fp);
        fp = NULL;
        printf("End: %s\n", file_path);

        free(file_path);
        file_path = NULL;
    }

    fclose(output_file);
    output_file = NULL;

    free(buffer);
    buffer = NULL;
}

size_t splitSentence(WcharPtr **sentences, WcharPtr content)
{
    WcharPtr ptr = NULL;
    WcharPtr *_sents = NULL;
    WcharPtr buffer = NULL;

    int chinese_char_cnt = 0;
    size_t max_sentences = 64;
    size_t sentence_cnt = 0;

    _sents = (WcharPtr *)malloc(sizeof(WcharPtr) * max_sentences);
    if (_sents == NULL)
    {
        printf("Sentences Malloc Error!\n");
        exit(1);
    }

    ptr = wcstok(content, tokens, &buffer);

    while (ptr != NULL)
    {

        WcharPtr test = ptr;
        if (isChinese(test[0]) == TRUE) // It is Chinese
        {

            chinese_char_cnt = countChineseChar(test);

            if (chinese_char_cnt > MIN_CHAR_LEN) // A sentence must be longer than 5.
            {
                _sents[sentence_cnt] = (WcharPtr)malloc(sizeof(wchar_t) * wcslen(test) + 2);
                if (_sents[sentence_cnt] == NULL)
                {
                    printf("sentence malloc error!\n");
                    exit(1);
                }
                wcscpy(_sents[sentence_cnt], test);
                sentence_cnt++;
            }
        }

        ptr = wcstok(NULL, tokens, &buffer); // next
    }

    /*---- Clean empty space ----*/
    WcharPtr *cleaned_sents = (WcharPtr *)malloc(sizeof(WcharPtr) * sentence_cnt);
    if (cleaned_sents == NULL)
    {
        printf("cleaned_sents malloc error!!\n");
        exit(1);
    }
    memcpy(cleaned_sents, _sents, sizeof(WcharPtr) * sentence_cnt);

    *sentences = cleaned_sents;

    return sentence_cnt;
}

int isChinese(const wchar_t first_c)
{
    unsigned long c = (unsigned long)first_c;

    int cond = (c >= MIN_CHINESE_CODE && c <= MAX_CHINESE_CODE);
    return (cond) ? TRUE : FALSE;
}

int countChineseChar(WcharPtr test)
{
    unsigned int chinese_char_cnt = 0;

    while (test != NULL && *test != L'\0')
    {
        if (isChinese(*test) == TRUE)
        {
            chinese_char_cnt++;
        }
        test++;
    }

    return chinese_char_cnt;
}

void outputOneNews(FILE *output_file, DataPtr data)
{
    size_t max_sent = data->sentence_count;
    for (size_t i = 0; i < max_sent; i++)
    {
        fwprintf(output_file, L"%ls#%ls#%ls\n", data->content[i], data->title, data->url);
    }
}

void sortOutputFile()
{
    FILE *fp;
    fp = fopen(output_file_name, "r");
    if (fp == NULL)
    {
        printf("sort open data.txr error!\n");
        exit(1);
    }

    FILE *final;
    final = fopen("final_data.txt", "w");
    if (final == NULL)
    {
        printf("open final error!\n");
        exit(1);
    }

    WcharPtr line = (WcharPtr)malloc(sizeof(wchar_t) * MAX_BUFFER_SIZE);
    if (line == NULL)
    {
        printf("line buffer malloc error!\n");
        exit(1);
    }

    WcharPtr *content = (WcharPtr *)malloc(sizeof(WcharPtr) * total_sentence_cnt);
    if (content == NULL)
    {
        printf("Content malloc Error!\n");
        exit(1);
    }

    size_t cnt = 0;

    while (fgetws(line, MAX_BUFFER_SIZE, fp) != NULL)
    {
        size_t line_len = wcslen(line);
        content[cnt] = (WcharPtr)malloc(sizeof(wchar_t) * line_len + 2);
        wcscpy(content[cnt], line);
        cnt++;
    }

    fclose(fp);
    fp = NULL;

    sort(content, cnt);

    for (size_t i = 0; i < cnt; i++)
    {
        fwprintf(final, L"%ls", content[i]);
    }

    fclose(final);
    final = NULL;

    free(content);
    content = NULL;

    free(line);
    line = NULL;
}