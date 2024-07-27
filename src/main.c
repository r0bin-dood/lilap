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

#include "lcu_log.h"
#include "cmd.h"
#include "lcu.h"
#include "lcu_tpool.h"

void install_handlers();
void exit_handler(int, siginfo_t *, void *);

void worker(void *arg)
{
    int *x = (int *)arg;
    printf("I'm worker %d\n", x);
    sleep(5);
}

int main(int argc, char **argv)
{
    IGNORE_RET(cmd_parse(argc, argv));

    install_handlers();

    lcu_tpool_t tpool = lcu_tpool_create(8);
    printf("tpool size: %lu\nworkers available: %lu\n", lcu_tpool_get_total_size(tpool), lcu_tpool_get_available_size(tpool));

    int x = 1;
    lcu_tpool_add_work(tpool, &worker, (void *)&x);
    printf("tpool size: %lu\nworkers available: %lu\n", lcu_tpool_get_total_size(tpool), lcu_tpool_get_available_size(tpool));
    int y = 2;
    lcu_tpool_add_work(tpool, &worker, (void *)&y);
    printf("tpool size: %lu\nworkers available: %lu\n", lcu_tpool_get_total_size(tpool), lcu_tpool_get_available_size(tpool));
    sleep(15);
    printf("tpool size: %lu\nworkers available: %lu\n", lcu_tpool_get_total_size(tpool), lcu_tpool_get_available_size(tpool));
    lcu_tpool_destroy(&tpool);

    return EXIT_SUCCESS;
}

void install_handlers()
{
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &exit_handler;
    if (sigaction(SIGTERM, &act, NULL) == -1)
    {
        printf("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        printf("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &act, NULL) == -1)
    {
        printf("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void exit_handler(int sig, siginfo_t *sig_info, void *context)
{
    UNUSED(sig);
    UNUSED(sig_info);
    UNUSED(context);
}
