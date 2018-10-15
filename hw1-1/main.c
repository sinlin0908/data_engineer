#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef struct d
{
    char *record_delimiter;
    char *key_pat;
    int case_insensitive;
    int reverse_order;
    int numerical_comparison;
} UserSetting;

UserSetting user_setting;

/** ==============
 * global variabel
==================*/
const int True = 1;
const int False = 0;

const size_t MAX_COMMAND_COUNT = 5;
const char *command_pattern[5] = {"-d", "-k", "-c", "-r", "-n"};

const size_t MAX_DEFAULT_ARG_COUNT = 2;
// -d must need delimiter, -k must need key_pat
char default_d_arg = '\n';
char default_k_arg = '\0';

/** ===================
 * Function Decalartion
=======================*/

void get_command(const int argc, char const **argv);
int find_command(const int argc, char const **argv, const char *command);
void set_user_setteing(const int argc, char const **argv);

size_t parse_file(const char *file_name, char ***records);

size_t span_array(char **ori, char *resour, size_t ori_size);
void store_record(char **record, char *resour);

void print_usage(void);
void print_error_message(void);
int comp_with_num(const void *a, const void *b);
int comp_with_str(const void *a, const void *b);

int main(int argc, char const *argv[])
{

    // UserSetting user_setting;
    const char *file_name = argv[1];
    char **records = NULL;
    size_t records_cnt = 0;

    get_command(argc, argv);
    records_cnt = parse_file(file_name, &records);

    clock_t start, end;
    if (user_setting.numerical_comparison != True) // no num
    {
        start = clock();
        qsort((void *)records, records_cnt, sizeof(char *), comp_with_str);
        end = clock();
    }
    else
    {
        start = clock();
        qsort((void *)records, records_cnt, sizeof(char *), comp_with_num);
        end = clock();
    }

    double duration = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Sort cost: %fsec\n", duration);

    FILE *f_out = fopen("result.txt", "w");
    for (size_t i = 0; i < records_cnt; i++)
    {
        fprintf(f_out, "%s\n", records[i]);
    }
    fclose(f_out);
    f_out = NULL;
    return 0;
}

void get_command(const int argc, char const **argv)
{
    /** Command
    * -d: record_delimiter
    * -k: key_pat
    * -c: case insensitive
    * -r: reverse order
    * -n: numerical comparison
    */

    if (argc < 2)
    {
        print_error_message();
    }
    else
    {
        set_user_setteing(argc, argv);
    }
}

void print_error_message(void)
{
    fprintf(stderr, "too few command!!\n");
    print_usage();
    exit(1);
}

void print_usage(void)
{
    printf("\nhelp:\n./main file_name -d delimiter (Need!) | -k key_pat | -c case insensitive | -r: reverse order | -n: numerical comparison\n");
}

void set_user_setteing(const int argc, char const **argv)
{
    int pos = find_command(argc, argv, command_pattern[0]);

    for (size_t i = 0; i < MAX_COMMAND_COUNT; i++)
    {
        int pos = find_command(argc, argv, command_pattern[i]);
        switch (i)
        {
        case 0:
            if (pos != -1 && pos + 1 != argc)
            {
                user_setting.record_delimiter = strdup(argv[pos + 1]);
                if (strcmp(user_setting.record_delimiter, "n") == 0)
                {
                    user_setting.record_delimiter[0] = default_d_arg;
                    user_setting.record_delimiter[1] = '\0';
                }
            }
            else
            {
                user_setting.record_delimiter = (char *)malloc(sizeof(char) * 2 + 1);
                user_setting.record_delimiter[0] = default_d_arg;
                user_setting.record_delimiter[1] = '\0';
            }

            break;
        case 1:
            if (pos != -1 && pos + 1 != argc)
            {
                user_setting.key_pat = strdup(argv[pos + 1]);
            }

            else
            {
                user_setting.key_pat = (char *)malloc(sizeof(char));
                user_setting.key_pat[0] = '\0';
            }
            break;
        case 2:
            if (pos != -1)
                user_setting.case_insensitive = True;
            else
                user_setting.case_insensitive = False;
            break;
        case 3:
            if (pos != -1)
                user_setting.reverse_order = True;

            else
                user_setting.reverse_order = False;

            break;
        case 4:
            if (pos != -1)
                user_setting.numerical_comparison = True;
            else
                user_setting.numerical_comparison = False;
            break;
        }
    }
}

int find_command(const int argc, char const **argv, const char *command)
{
    for (size_t idex = 0; idex < argc; idex++)
    {
        if (strcmp(argv[idex], command) == 0)
        {
            return idex;
        }
    }
    return -1;
}

size_t parse_file(const char *file_name, char ***records)
{
    char *delimiter = user_setting.record_delimiter;
    const size_t delimiter_len = strlen(delimiter);

    FILE *f_in = fopen(file_name, "r");
    if (f_in == NULL)
    {
        fprintf(stderr, "fin open error!!\n");
        exit(1);
    }

    size_t max_buffer_size = 10240;
    char *buffer = (char *)malloc(sizeof(char) * max_buffer_size + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "buffer malloc error!\n");
        exit(1);
    }

    size_t max_record_size = 1024;
    char **records_ = (char **)malloc(sizeof(char *) * max_record_size + 1);
    if (records_ == NULL)
    {
        fprintf(stderr, "records_ malloc error!\n");
        exit(1);
    }

    char *temp = NULL;
    size_t temp_size = 0;

    int file_count = 0;
    size_t records_cnt = 0;

    while (fgets(buffer, max_buffer_size, f_in) != NULL)
    {
        /* Span records_ */
        if (records_cnt > max_record_size)
        {
            max_record_size *= 2;
            char **temp_ = (char **)malloc(sizeof(char *) * max_record_size);
            if (temp_ == NULL)
            {
                fprintf(stderr, "span records malloc error!\n");
                exit(1);
            }
            memcpy(temp_, records_, sizeof(char *) * records_cnt);
            free(records_);
            records_ = temp_;
            temp_ = NULL;
        }

        size_t buffer_len = strlen(buffer);
        if (buffer[buffer_len - 1] == EOF && file_count < 5)
        {
            file_count++;
            buffer[buffer_len - 1] = '\0';
        }

        /* find whether delimiter is in buffer */
        char *ptr = strstr(buffer, delimiter);

        if (ptr != NULL)
        {
            char *now = buffer;
            char *next;

            /*  find delimiter instantly */
            while ((next = strstr(now, user_setting.record_delimiter)) != NULL && *now != '\0')
            {
                // printf("now %s", now);
                // found!! refresh temp
                // store record
                if (temp_size != 0)
                {
                    store_record(&records_[records_cnt], temp);
                    // printf("ss %s", temp);
                    records_cnt++;
                    temp_size = 0;
                    free(temp);
                    temp = NULL;
                }

                size_t len = next - now; // len > 0 has next delimiter
                if (len > 0)
                {
                    char *t = strndup(now, len);
                    // store record
                    // printf("ss %s\n", t);
                    store_record(&records_[records_cnt], t);
                    records_cnt++;
                    free(t);
                    t = NULL;
                    temp_size = 0;
                }
                now = next + delimiter_len + 1;

                // next = next + delimiter_len;
                // now = next;
            }
            if (*now != '\0')
            {
                // store temp
                temp_size = span_array(&temp, now, temp_size); // err
            }
        }
        else
        {
            // store temp
            temp_size = span_array(&temp, buffer, temp_size);
        }
        for (size_t i = 0; i < max_buffer_size; i++)
        {
            buffer[i] = '\0';
        }
    }

    store_record(&records_[records_cnt], temp);
    records_cnt++;

    free(temp);
    temp = NULL;

    free(buffer);
    buffer = NULL;
    free(delimiter);
    delimiter = NULL;

    fclose(f_in);
    f_in = NULL;

    *records = records_;

    return records_cnt;
}

size_t span_array(char **ori, char *resour, size_t ori_size)
{
    if (*ori == NULL)
    {
        *ori = strdup("\0");
        ori_size += strlen(*ori);
    }

    size_t size = ori_size + strlen(resour) + 1;
    char *temp = (char *)malloc(sizeof(char) * size);
    if (temp == NULL)
    {
        fprintf(stderr, "span_array malloc error!\n");
        exit(1);
    }

    strcpy(temp, *ori);
    strcat(temp, resour);
    free(*ori);
    *ori = NULL;
    *ori = temp;

    return size;
}

void store_record(char **record, char *resour)
{
    char *temp = strdup(resour);
    *record = temp;
}

int comp_with_num(const void *a, const void *b)
{
    const char *a_ = *(const char **)a;
    const char *b_ = *(const char **)b;
    int num1, num2;

    if (user_setting.key_pat[0] != '\0')
    {
        a_ = strstr(a_, user_setting.key_pat); // start from key pattern
        b_ = strstr(b_, user_setting.key_pat);
    }

    while (*a_ != '\0' && a_ != NULL && !isdigit(*a_))
    {
        a_++;
    }
    num1 = atoi(a_);

    while (*b_ != '\0' && b_ != NULL && !isdigit(*b_))
    {
        b_++;
    }
    num2 = atoi(b_);

    int d = num1 - num2;

    if (user_setting.reverse_order)
    {
        return -d;
    }
    else
    {
        return d;
    }
}

int comp_with_str(const void *a, const void *b)
{
    const char *a_ = *(const char **)a;
    const char *b_ = *(const char **)b;
    if (user_setting.key_pat[0] != '\0')
    {
        a_ = strstr(a_, user_setting.key_pat); // start from key pattern
        b_ = strstr(b_, user_setting.key_pat);
    }

    if (user_setting.case_insensitive == True)
    {
        if (user_setting.reverse_order == True)
            return -1 * strcasecmp(a_, b_);
        else
            return strcasecmp(a_, b_);
    }
    else
    {
        if (user_setting.reverse_order == True)
            return -1 * strcmp(a_, b_);
        else
            return strcmp(a_, b_);
    }
}