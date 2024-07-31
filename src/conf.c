#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "lcu.h"
#include "lcu_logger.h"
#include "conf.h"

static void helper_extract_property_and_value(char *property_value_pair, char **property, char **value);
static bool helper_compare_property(const char *s1, const char *s2);
static void helper_set_str_property(char **dest, char *src, size_t len);
static void helper_set_int_property(size_t *dest, char *src);

void conf_parse(conf_t *conf, const char *file)
{
    FILE *fd = fopen(file, "r");
    if (fd == NULL)
        return;

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, fd) != -1)
    {
        if (line[0] == '#') // it's commented out, stick with the default
            continue;
        
        char *property = NULL;
        char *value = NULL;
        helper_extract_property_and_value(line, &property, &value);

        if (helper_compare_property(stringify(conf->log_dir), property))
        {
            if ((strcmp(value, LCU_STDOUT) == 0) || (strcmp(value, LCU_STDERR) == 0))
                continue;

            pid_t pid = getpid();
            char log_file_path[128];
            sprintf(log_file_path, "%s/lilap.%d", value, pid);

            helper_set_str_property(&conf->log_dir, log_file_path, strlen(log_file_path));
        }
        else if (helper_compare_property(stringify(conf->ap_name), property))
        {
            helper_set_str_property(&conf->ap_name, value, strlen(value));
        }
        else if (helper_compare_property(stringify(conf->tpool_threads), property))
        {
            helper_set_int_property(&conf->tpool_threads, value);
        }
    }

    free(line);

    fclose(fd);
}

void helper_extract_property_and_value(char *property_value_pair, char **property, char **value)
{
    *property = property_value_pair;

    while (*(property_value_pair++) != ' ');
    *value = property_value_pair;

    *(--property_value_pair) = '\0';

    while (*(property_value_pair++) != '\n');
    *(--property_value_pair) = '\0';
}

static bool helper_compare_property(const char *s1, const char *s2)
{
    while (*(s1++) != '>');
    const char *struct_member_as_str = s1;

    return (strcmp(struct_member_as_str, s2) == 0);
}

void helper_set_str_property(char **dest, char *src, size_t len)
{
    *dest = (char *) malloc(sizeof(char) * (len + 1));
    memcpy(*dest, src, len);
    (*dest)[len] = '\0';
}

void helper_set_int_property(size_t *dest, char *src)
{
    *dest = (size_t) strtoul(src, NULL, 10);
}
