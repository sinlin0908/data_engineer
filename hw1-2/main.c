#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

typedef enum bool
{
    false,
    true
} bool;

typedef struct
{
    FILE **temp_files;
    size_t temp_files_count;
} FileDetail;

typedef struct
{
    char *data;
    int pos;
} HeapElement;

typedef struct
{
    bool d, k, s;
    char *key, *limit_mem, *d_para;
} Parameter;

typedef struct
{
    clock_t start;
    clock_t end;
} Time;

/**================
*  Function Declare
===================*/

FILE *openFile(char *file_name, char *mode);

void writeToFile(FILE *output_file, char **records, size_t count);

int cmp(const void *a, const void *b);

char *mallocStringSpace(size_t size);

char **mallocStringArraySpace(size_t size);

FILE **mallocFilesSpace(size_t size);

size_t expandStringSpace(char **str, size_t now_len, size_t max_size);

size_t expandStringArraySpace(char ***str_array, size_t now_len, size_t max_size);

void resetStringSpace(char **str, size_t size);

//-------------------------------------------------

void getCommand(int argc, char const **argv, char *fin_name);
int findCommand(int argc, char const **argv, char *command);

void splitandSortFile(FILE *f_in, FileDetail *temp_files_detail);
/**/

char *findDPara(char *target, char *pattern);
/**/

size_t storeOneRecord(char ***records, char **one_record, size_t index);

/**
 * Global Variable
*/
Parameter param = {false, false, false, NULL, NULL};

char *command[] = {"-d", "-k", "-s", "-m"};
const unsigned int MAX_COMMAND_COUNT = 4;
const size_t MAX_BUFFER_SIZE = 1024;
char *final_file_name = "final_file.txt";
char *temp_file_head = "temp_";
//-----------

int main(int argc, char const *argv[])
{

    FILE *f_in;
    char fin_name[30];
    FileDetail temp_files_detail;

    getCommand(argc, argv, fin_name);

    printf("d parameter:%s\n", param.d_para);
    printf("d parameter len: %ld\n", strlen(param.d_para));

    f_in = openFile(fin_name, "r");
    printf("Start to split and sort temp files....\n\n");
    splitandSortFile(f_in, &temp_files_detail);
    fclose(f_in);
    f_in = NULL;

    // printf("Start to merge temp files.....\n\n")l
    // mergeTempFiles(&temp_files_detail);

    return 0;
}

void getCommand(int argc, char const **argv, char *file_name)
{

    if (argc < 2)
    {
        fprintf(stderr, "No Open File!!\n");
        exit(1);
    }

    // get file name
    strcpy(file_name, argv[1]);

    // get parameter
    for (int i = 0; i < MAX_COMMAND_COUNT; i++)
    {
        int index = findCommand(argc, argv, command[i]);
        if (index != -1)
        {
            switch (i)
            {
            case 0:
                param.d = true;
                if (index + 1 >= argc || strcmp(argv[index + 1], "n") == 0)
                {
                    param.d_para = mallocStringSpace(2);
                    param.d_para[0] = '\n';
                    param.d_para[1] = '\0';
                }
                else
                {
                    param.d_para = strdup(argv[index + 1]);
                }

                break;
            case 1:
                param.k = true;
                if (index + 1 >= argc)
                {
                    param.key = mallocStringSpace(2);
                    param.key[0] = '\n';
                    param.key[1] = '\0';
                }
                else
                {
                    param.key = strdup(argv[index + 1]);
                }
                break;
            case 2:
                param.s = true;
                break;
            case 3:
                param.limit_mem = strdup(argv[index + 1]);
                break;
            }
        }
    }
}

int findCommand(int argc, char const **argv, char *command)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], command) == 0)
        {
            return i;
        }
    }

    return -1;
}

void splitandSortFile(FILE *f_in, FileDetail *temp_files_detail)
{
    // Memory
    int user_mem = (param.limit_mem == NULL) ? 4096 : atoi(param.limit_mem);

    printf("user_mem %dM\n====================\n\n", user_mem);
    if (user_mem > 4096)
    {
        fprintf(stderr, "memory too large!!\n");
        exit(1);
    }

    unsigned long long int MAX_MEM_USE_SIZE = (unsigned long long int)user_mem * 1024 * 1024;
    size_t memory_use_size = 0;

    // out files
    size_t out_files_size = 5;
    size_t out_files_count = 0;
    FILE **out_files = mallocFilesSpace(out_files_size);

    // read file
    size_t record_size = MAX_BUFFER_SIZE;

    size_t records_size = 1024;
    size_t records_cnt = 0;
    char **records_ = mallocStringArraySpace(records_size);

    char buffer[MAX_BUFFER_SIZE];

    char *one_record = mallocStringSpace(record_size);
    size_t current_one_record_size = 0;
    one_record[0] = '\0';
    char temp_file_name[20];

    while (fgets(buffer, MAX_BUFFER_SIZE, f_in) != NULL)
    {
        size_t buffer_len = strlen(buffer);

        char *d_ptr = NULL;
        if ((d_ptr = findDPara(buffer, param.d_para)) != NULL)
        {
            if (memory_use_size + buffer_len >= MAX_MEM_USE_SIZE)
            {
                // sort records and write to file
                // mem use -= len(one_record)
                printf("mem not enough, create temp file\n");
                qsort((void *)records_, records_cnt, sizeof(char *), cmp);
                // FILE *tempfile = tmpfile();
                sprintf(temp_file_name, "temp_%ld.txt", out_files_count);
                FILE *tempfile = openFile(temp_file_name, "w");
                writeToFile(tempfile, records_, records_cnt);

                for (size_t i = 0; i < records_cnt; i++)
                {
                    memset(records_[i], 0, strlen(records_[i]));
                }

                memset(temp_file_name, 0, strlen(temp_file_name));

                out_files[out_files_count] = tempfile;
                tempfile = NULL;
                out_files_count++;

                memory_use_size = strlen(one_record);
                printf("new mem size %ld\n", strlen(one_record));

                records_cnt = 0;
            }
            // store word before d_para to one_record
            char *head = buffer;
            size_t pre_len = d_ptr - head;
            if (pre_len != 0)
            {
                strncat(one_record, buffer, pre_len);
                current_one_record_size += pre_len;
                memory_use_size += pre_len;
            }
            // start store records
            if (records_cnt >= records_size)
            {
                records_size = expandStringArraySpace(&records_, records_cnt, records_size);
            }

            records_[records_cnt++] = strdup(one_record);

            resetStringSpace(&one_record, record_size);
            current_one_record_size = 0;

            // store word after d_para to one record
            char *ptr = head + pre_len;
            size_t end_len = strlen(ptr);
            strcat(one_record, ptr);
            current_one_record_size += end_len;
            memory_use_size += end_len;
        }
        else
        {
            // store one record
            if (current_one_record_size + buffer_len >= record_size)
            {
                record_size = expandStringSpace(&one_record, current_one_record_size, record_size);
            }

            if (memory_use_size + buffer_len >= MAX_MEM_USE_SIZE)
            {
                // sort records and write to file
                // mem use -= len(one_record)
                printf("mem not enough, create temp file\n");
                qsort((void *)records_, records_cnt, sizeof(char *), cmp);
                // FILE *tempfile = tmpfile();
                sprintf(temp_file_name, "temp_%ld.txt", out_files_count);
                FILE *tempfile = openFile(temp_file_name, "w");
                writeToFile(tempfile, records_, records_cnt);

                for (size_t i = 0; i < records_cnt; i++)
                {
                    memset(records_[i], 0, strlen(records_[i]));
                }

                memset(temp_file_name, 0, strlen(temp_file_name));

                out_files[out_files_count] = tempfile;
                tempfile = NULL;
                out_files_count++;

                memory_use_size = strlen(one_record);
                printf("new mem size %ld\n", strlen(one_record));

                records_cnt = 0;
            }

            strcat(one_record, buffer);
            current_one_record_size += buffer_len;
            memory_use_size += buffer_len;
        }
    }
    // final
    records_[records_cnt++] = strdup(one_record);
    qsort((void *)records_, records_cnt, sizeof(char *), cmp);

    // FILE *temp_file = tmpfile();
    sprintf(temp_file_name, "temp_%ld.txt", out_files_count);
    FILE *temp_file = openFile(temp_file_name, "w");
    writeToFile(temp_file, records_, records_cnt);

    out_files[out_files_count] = temp_file;
    temp_file = NULL;
    out_files_count++;

    for (int i = 0; i < records_cnt; i++)
    {
        free(records_[i]);
        records_[i] = NULL;
    }

    memset(temp_file_name, 0, strlen(temp_file_name));

    free(records_);
    records_ = NULL;
    free(one_record);
    one_record = NULL;
    free(out_files);
    out_files = NULL;

    temp_files_detail->temp_files_count = out_files_count;
    temp_files_detail->temp_files = out_files;
}

void mergeTempFiles(FileDetail *file_detail)
{
    FILE *out_file = openFile(final_file_name, "w");

    size_t K = file_detail->temp_files_count;
    size_t temp_file_count = file_detail->temp_files_count;

    FILE *tmp_files[temp_file_count];
    char file_name[20];

    char read_buffer[MAX_BUFFER_SIZE];

    unsigned long long int memory_size = atoll(param.limit_mem) * 1024 * 1024;

    // size_t max_line_size = (size_t)(memory_size / (long long int)temp_file_count);

    // char ***files_buffer = (char **)malloc(sizeof(char **) * temp_file_count);
    // if (files_buffer == NULL)
    // {
    //     fprintf(stderr, "merge buffer malloc error!\n");
    //     exit(1);
    // }

    for (size_t i = 0; i < temp_file_count; i++)
    {
        // files_buffer[i] = mallocStringArraySpace(max_line_size);
        sprintf(file_name, "temp_%ld.txt", i);
        tmp_files[i] = openFile(file_name, "r");

        while (fgets(read_buffer, MAX_BUFFER_SIZE, tmp_files[i] != NULL))
        {
        }
    }

    fclose(out_file);
    out_file = NULL;
}

char *findDPara(char *target, char *pattern)
{
    char *ptr = strstr(target, pattern);

    return (ptr != NULL) ? ptr : NULL;
}

void writeToFile(FILE *output_file, char **records, size_t count)
{
    if (output_file == NULL)
    {
        output_file = stdout;
    }
    for (int i = 0; i < count; i++)
    {
        fprintf(output_file, "%s-----------\n", records[i]);
        // printf("%s\n", records[i]);
        // printf("%ld\n", strlen(records[i]));
    }
}

int cmp(const void *a, const void *b)
{
    const char *str1 = *(const char **)a;
    const char *str2 = *(const char **)b;

    if (param.s == true)
    {
        return strlen(str1) - strlen(str2);
    }
    else
    {
        char *ptr1 = strstr(str1, param.d_para);
        if (ptr1 == NULL)
        {
            fprintf(stderr, "strcmp error!\n");
            exit(1);
        }
        char *ptr2 = strstr(str2, param.d_para);
        if (ptr1 == NULL)
        {
            fprintf(stderr, "strcmp error!\n");
            exit(1);
        }

        return strcmp(ptr1, ptr2);
    }
}

/**====================
 *      Util
=======================*/

FILE *openFile(char *file_name, char *mode)
{
    FILE *f;
    printf("Open File: %s\n\n", file_name);

    f = fopen(file_name, mode);
    if (f == NULL)
    {
        fprintf(stderr, "Open File Error!\n");
        exit(1);
    }

    return f;
}

char *mallocStringSpace(size_t size)
{
    char *temp = (char *)malloc(sizeof(char) * size + 1);
    if (temp == NULL)
    {
        fprintf(stderr, "String malloc Error");
        exit(1);
    }

    return temp;
}

char **mallocStringArraySpace(size_t size)
{
    char **temp = (char **)malloc(sizeof(char *) * size + 1);
    if (temp == NULL)
    {
        fprintf(stderr, "String array malloc Error!\n");
        exit(1);
    }

    return temp;
}

FILE **mallocFilesSpace(size_t size)
{
    FILE **temp = (FILE **)malloc(sizeof(FILE *) * size);
    if (temp == NULL)
    {
        fprintf(stderr, "malloc file space error!!\n");
        exit(1);
    }

    return temp;
}

size_t expandStringSpace(char **str, size_t now_len, size_t max_size)
{
    // printf("---- expand string! ----\n");
    size_t new_max_size = max_size * 2;

    char *temp = (char *)malloc(sizeof(char) * new_max_size + 1);
    if (temp == NULL)
    {
        fprintf(stderr, "expand string space malloc error!\n");
        exit(1);
    }

    memcpy(temp, *str, sizeof(char) * now_len);
    if (*str != NULL)
    {
        free(*str);
        *str = NULL;
    }
    *str = temp;
    temp = NULL;

    return new_max_size;
}

size_t expandStringArraySpace(char ***str_array, size_t now_len, size_t max_size)
{
    // printf("---- expand string array! ----\n");
    size_t new_size = max_size * 2;

    char **temp = (char **)malloc(sizeof(char *) * new_size + 2);
    if (temp == NULL)
    {
        fprintf(stderr, "expand string array malloc error!\n");
        exit(1);
    }

    memcpy(temp, *str_array, sizeof(char *) * now_len);
    free(*str_array);
    *str_array = temp;
    temp = NULL;

    return new_size;
}

void resetStringSpace(char **str, size_t size)
{
    // printf("---- reset string space ----\n");
    // char *temp = NULL;

    // temp = mallocStringSpace(size);
    // temp[0] = '\0';
    if (*str != NULL)
    {
        memset(*str, 0, strlen(*str));
        // free(*str);
        // *str = NULL;
    }
    // *str = temp;
    // temp = NULL;
}

void adjustHeap()
{
}
void createHeap(size_t size)
{
}
