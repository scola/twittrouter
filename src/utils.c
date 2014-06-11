#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "twittrouter.h"

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
    for (count = 0; (tmp = strstr(ins, rep)) != NULL; ++count) {
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
    printf("    Firstly,test whether the default oauth works normally\n");
    printf("    Try to run [twittrouter -u kfc]\n");
    printf("    Second,authorize your own twitter account\n");
    printf("    Try to run [twittrouter -a]\n");
    printf("    If the two step above works normally,then run [twittrouter]\n");
    printf("    Optional,you can add your own device mac address into whitelist of /etc/config/twittrouter.json\n");
    printf("\n");
    printf("    [-p <servPort>]       server port,the default value is 9999\n");
    printf("    [-c <config path>]    twittrouter.json path,default /etc/config/twittrouter.json\n");
    printf("    [-r <root>]           html file path for the server,default /www/twittrouter\n");
    printf("    [-h <help>]           get the usage of the twittrouter\n");
    printf("    [-u <username>]       just use to test the oauth and network config\n");
    printf("    [-f <pid_file>]       valid path to the pid file\n");
    printf("    [-a <authorize>]      authorize and get twitter acess token\n");
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

