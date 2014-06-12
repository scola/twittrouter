#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "jconf.h"
#include "json.h"
#include "string.h"

static char *to_string(const json_value *value)
{
    if (value->type == json_string)
    {
        return ss_strndup(value->u.string.ptr, value->u.string.length);
    }
    else if (value->type == json_integer)
    {
#ifdef __MINGW32__
        return strdup(ss_itoa(value->u.integer));
#else
        return strdup(itoa(value->u.integer));
#endif
    }
    else if (value->type == json_null)
    {
        return "null";
    }
    else
    {
        LOGE("%d", value->type);
        FATAL("Invalid config format.");
    }
    return 0;
}

static char* packstring(char *origin, char *delm) {
    int packed_len = strlen(origin) + 2 * strlen(delm) + 1;
    char *ret = (char *)malloc(packed_len);
    memset(ret,0,packed_len);
    strcat(ret,delm);
    strcat(ret,origin);
    strcat(ret,delm);
    return ret;
}

static char* concatstring(char *origin, char *delm, char *tail) {
    int concat_len = strlen(origin) + strlen(delm) + strlen(tail) + 1;
    char *ret = (char *)malloc(concat_len);
    memset(ret,0,concat_len);
    strcat(ret,origin);
    strcat(ret,delm);
    strcat(ret,tail);
    return ret;
}

static char* build_json_item(char *key, char *value) {
    char *pack_key = packstring(key, "\"");
    char *pack_value = packstring(value, "\"");
    char *ret = concatstring(pack_key, ":", pack_value);
    if (pack_key) free(pack_key);
    if (pack_value) free(pack_value);
    return ret;
}

static int dump_to_file(const char *buffer, size_t size, char *file)
{
    FILE *dest = fopen(file, "w");
    if(fwrite(buffer, size, 1, dest) != 1)
        return -1;
    return 0;
}

jconf_t *read_jconf(const char* file)
{

    static jconf_t conf;

    char *buf;
    json_value *obj;

    FILE *f = fopen(file, "r");
    if (f == NULL) FATAL("Invalid config path.");

    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (pos >= MAX_CONF_SIZE) FATAL("Too large config file.");

    buf = malloc(pos + 1);
    if (buf == NULL) FATAL("No enough memory.");

    fread(buf, pos, 1, f);
    fclose(f);

    buf[pos] = '\0'; // end of string

    json_settings settings = { 0 };
    char error_buf[512];
    obj = json_parse_ex(&settings, buf, pos, error_buf);

    if (obj == NULL)
    {
        FATAL(error_buf);
    }

    if (obj->type == json_object)
    {
        int i;
        for (i = 0; i < obj->u.object.length; i++)
        {
            char *name = obj->u.object.values[i].name;
            json_value *value = obj->u.object.values[i].value;
            if (strcmp(name, "whitelist") == 0)
            {
                conf.whitelist = to_string(value);
            }
            else if (strcmp(name, "TwitterID") == 0)
            {
                conf.TwitterID = to_string(value);
            }
            else if (strcmp(name, "CONSUMER_KEY") == 0)
            {
                conf.CONSUMER_KEY = to_string(value);
            }
            else if (strcmp(name, "CONSUMER_SECRET") == 0)
            {
                conf.CONSUMER_SECRET = to_string(value);
            }
            else if (strcmp(name, "OAUTH_TOKEN") == 0)
            {
                conf.OAUTH_TOKEN = to_string(value);
            }
            else if (strcmp(name, "OAUTH_TOKEN_SECRET") == 0)
            {
                conf.OAUTH_TOKEN_SECRET = to_string(value);
            }
        }
    }
    else
    {
        FATAL("Invalid config file");
    }

    free(buf);
    json_value_free(obj);
    return &conf;

}

void dump_jconf(char *conf_path){
    char* jsonlist[6];
    extern jconf_t *conf;
    //jsonlist[0] = concatstring(packstring("TwitterID", "\""), ":" ,packstring(conf->whitelist, "\""));
    jsonlist[0] = build_json_item("TwitterID",conf->TwitterID);
    jsonlist[1] = build_json_item("CONSUMER_KEY",conf->CONSUMER_KEY);
    jsonlist[2] = build_json_item("CONSUMER_SECRET",conf->CONSUMER_SECRET);
    jsonlist[3] = build_json_item("OAUTH_TOKEN",conf->OAUTH_TOKEN);
    jsonlist[4] = build_json_item("OAUTH_TOKEN_SECRET",conf->OAUTH_TOKEN_SECRET);
    jsonlist[5] = build_json_item("whitelist",conf->whitelist);
    
    int j;
    int stringlen = 0;
    for(j = 0; j < 6; j++) {
        stringlen += strlen(jsonlist[j]);
    }
    int total_len = stringlen + 5 * 4 + 1;
    char *all_item = (char *)malloc(total_len);
    memset(all_item,0, total_len);
    
    for(j = 0; j < 5; j++) {
        strcat(all_item,jsonlist[j]);
        strcat(all_item,",\r\n\t");
    }
    strcat(all_item,jsonlist[5]);
    
    for(j = 0; j < 6; j++) {
        if (jsonlist[j]) free(jsonlist[j]);
    }
    
    char *final_string = (char *)malloc(total_len + 4 + 3 + 1);
    memset(final_string, 0, total_len + 4 + 3 + 1);
    strcat(final_string, "{\r\n\t");
    strcat(final_string, all_item);
    strcat(final_string, "\r\n}");
    
    if(all_item) free(all_item);
    printf("\n%s\n", final_string);
    dump_to_file(final_string, strlen(final_string), conf_path);
    if(final_string) free(final_string);
}
