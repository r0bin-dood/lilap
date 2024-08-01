#include <asm/types.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <time.h>
#include "lcu.h"
#include "lcu_logger.h"
#include "lcu_tpool.h"
#include "conf.h"

static bool got_exit;
static pthread_cond_t exit_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exit_mutex = PTHREAD_MUTEX_INITIALIZER;

static void install_handlers();
static void exit_handler(int, siginfo_t *, void *);
static void wait_for_exit();

int main(int argc, const char **argv)
{
    if (argc != 2)
    {
        printf("Usage: lilap <conf_file>\n");
        exit(EXIT_FAILURE);
    }

    conf_t *conf = get_conf();

    conf_parse(conf, argv[1]);
        
    lcu_logger_create(conf->log_dir);

    lcu_logger_print("hello world\n");
    lcu_logger_print("\"%s\" %lu\n", conf->ap_name, conf->tpool_threads);

    install_handlers();
    
    wait_for_exit();

    lcu_logger_destroy();

    return EXIT_SUCCESS;
}

void install_handlers()
{
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &exit_handler;
    if (sigaction(SIGTERM, &act, NULL) == -1)
    {
        lcu_logger_print("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        lcu_logger_print("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &act, NULL) == -1)
    {
        lcu_logger_print("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void exit_handler(int sig, siginfo_t *sig_info, void *context)
{
    UNUSED(sig);
    UNUSED(sig_info);
    UNUSED(context);

    pthread_mutex_lock(&exit_mutex);
    got_exit = true;
    pthread_cond_signal(&exit_cond);
    pthread_mutex_unlock(&exit_mutex);
}

void wait_for_exit()
{
    pthread_mutex_lock(&exit_mutex);
    while (got_exit == false)
        pthread_cond_wait(&exit_cond, &exit_mutex);
    pthread_mutex_unlock(&exit_mutex);
}