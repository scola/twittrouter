#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "twittrouter.h"

#define VERSION "0.1.0"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define INT_DIGITS 19		/* enough for 64 bit integer */


#ifndef __MINGW32__
void ERROR(const char *s)
{
    char *msg = strerror(errno);
    LOGE("%s: %s", s, msg);

}
#endif

#ifdef __MINGW32__
char *ss_itoa(int i)
#else
char *itoa(int i)
#endif
{
    /* Room for INT_DIGITS digits, - and '\0' */
    static char buf[INT_DIGITS + 2];
    char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
    if (i >= 0)
    {
        do
        {
            *--p = '0' + (i % 10);
            i /= 10;
        }
        while (i != 0);
        return p;
    }
    else  			/* i < 0 */
    {
        do
        {
            *--p = '0' - (i % 10);
            i /= 10;
        }
        while (i != 0);
        *--p = '-';
    }
    return p;
}

char *ss_strndup(const char *s, size_t n)
{
    size_t len = strlen(s);
    char *ret;

    if (len <= n) return strdup(s);

    ret = malloc(n + 1);
    strncpy(ret, s, n);
    ret[n] = '\0';
    return ret;
}

void FATAL(const char *msg)
{
    LOGE("%s", msg);
    exit(-1);
}

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

void usage()
{
    printf("\n");
    printf("twittrouter --version %s\n\n", VERSION);
    printf("  maintained by Scola <shaozheng.wu@gmail.com>\n\n");
    printf("  usage:\n\n");
    printf("    please edit your config file in /etc/config/twittrouter.json firstly\n");
    printf("    you must create your own twitter app(https://dev.twitter.com/apps)\n");
    printf("    and add the Consumer key,secret Access token,secret in the twittrouter.json\n");
    printf("\n");
    printf("    TwitterID:            your twitter username\n");
    printf("    CONSUMER_KEY:         your twitter app consumer key\n");
    printf("    CONSUMER_SECRET:      your twitter app consumer secret\n");
    printf("    OAUTH_TOKEN:          your twitter app authorization token\n");
    printf("    OAUTH_TOKEN_SECRET:   your twitter app authorization token secret\n");
    printf("    whitelist:            your own device mac address that split by '|'\n");
    printf("\n");
    printf("    [-p <servPort>]       server port,the default value is 9999\n");
    printf("    [-h <help>]           get the usage of the twittrouter\n");
    printf("    [-v <version>]        the twittrouter version\n");
    printf("\n");
}

void demonize(const char* path)
{
#ifndef __MINGW32__
    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0)
    {
        FILE *file = fopen(path, "w");
        if (file == NULL) FATAL("Invalid pid file\n");

        fprintf(file, "%d", pid);
        fclose(file);
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Open any logs here */

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0)
    {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0)
    {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
#endif
}

