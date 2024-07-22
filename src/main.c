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

#include "utils.h"
#include "logger.h"
#include "cmd.h"

static uint8_t send_buf[4096];
static uint8_t recv_buf[4096];

bool got_exit = false;

void install_handlers();
void exit_handler(int, siginfo_t *, void *);

int send_msg(int sock_fd, struct sockaddr_nl *);

int main(int argc, char **argv)
{
    IGNORE_RET(cmd_parse(argc, argv));

    install_handlers();

    int ret = 0;
    int rtnl_soc_fd = 0;
    struct sockaddr_nl rtnl_sockaddr;

    rtnl_soc_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
    if (rtnl_soc_fd == -1) 
    {
        log_error("socket: %d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    IGNORE_RET(memset(&rtnl_sockaddr, 0, sizeof(rtnl_sockaddr)));
    rtnl_sockaddr.nl_family = AF_NETLINK;
    rtnl_sockaddr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
    ret = bind(rtnl_soc_fd, (struct sockaddr *)&rtnl_sockaddr, sizeof(rtnl_sockaddr));
    if (ret == -1)
    {
        log_error("bind: %d: %s", errno, strerror(errno));
        close(rtnl_soc_fd);
        exit(EXIT_FAILURE);
    }

    memset(send_buf, 0, 1024);

    struct nlmsghdr *nh = (struct nlmsghdr *)&send_buf;

    nh->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));

    struct iovec iov;
    memset(&iov, 0, sizeof(struct iovec));
    iov.iov_base = nh;
    iov.iov_len = nh->nlmsg_len;

    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    msg.msg_name = &rtnl_sockaddr;
    msg.msg_namelen = sizeof(struct sockaddr_nl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;

    nh->nlmsg_pid = getpid();
    nh->nlmsg_seq = 1;
    nh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nh->nlmsg_type = RTM_GETLINK;

    struct ifinfomsg payload;
    memset(&payload, 0, sizeof(struct ifinfomsg));
    payload.ifi_family = AF_UNSPEC;
    payload.ifi_change = 0xFFFFFFFF;

    memcpy(NLMSG_DATA(nh), &payload, sizeof(payload));

    int len = 0;
    len = sendmsg(rtnl_soc_fd, &msg, 0);
    if (len == -1)
    {
        log_error("sendmsg: %d: %s", errno, strerror(errno));
        close(rtnl_soc_fd);
        exit(EXIT_FAILURE);
    }
    
    struct msghdr msg_r;
    struct iovec iov_r;
    iov_r.iov_base = recv_buf;
    iov_r.iov_len = sizeof(recv_buf);

    msg_r.msg_name = &rtnl_sockaddr;
    msg_r.msg_namelen = sizeof(struct sockaddr_nl);
    msg_r.msg_iov = &iov_r;
    msg_r.msg_iovlen = 1;
    msg_r.msg_control = NULL;
    msg_r.msg_controllen = 0;
    msg_r.msg_flags = 0;

    int recv_len = 0;
    recv_len = recvmsg(rtnl_soc_fd, &msg_r, 0);
    if (recv_len == -1)
    {
        log_error("recvmsg: %d: %s", errno, strerror(errno));
        close(rtnl_soc_fd);
        exit(EXIT_FAILURE);
    }

    for (nh = (struct nlmsghdr *)&recv_buf; NLMSG_OK(nh, iov_r.iov_len); nh = NLMSG_NEXT(nh, iov_r.iov_len))
    {
        struct ifinfomsg *t = (struct ifinfomsg *)NLMSG_DATA(nh);
        printf("\t %d:\n", t->ifi_index);
            printf("\t\tifi_family: %d\n", t->ifi_family);
            printf("\t\tifi_type:   %d\n", t->ifi_type);
            printf("\t\tifi_index:  %d\n", t->ifi_index);
            printf("\t\tifi_flags:  %d\n", t->ifi_flags);
            printf("\t\tifi_change: %d\n", t->ifi_change);
        struct rtattr *rta;
        rta = (struct rtattr *) IFLA_RTA(t);
        int attr_len = IFLA_PAYLOAD(nh);

        for (; RTA_OK(rta, attr_len); rta = RTA_NEXT(rta, attr_len))
        {
            switch (rta->rta_type)
            {
                case IFLA_ADDRESS:
                {
                    unsigned char *mac_buf = (unsigned char *)RTA_DATA(rta);
                    printf("\t\taddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            mac_buf[0], mac_buf[1], mac_buf[2],
                            mac_buf[3], mac_buf[4], mac_buf[5]);
                    break;
                }
                case IFLA_IFNAME:
                {
                    printf("\t\tifname: %s\n", (char *)RTA_DATA(rta));
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

void install_handlers()
{
    struct sigaction act;

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &exit_handler;
    if (sigaction(SIGTERM, &act, NULL) == -1)
    {
        log_error("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        log_error("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGABRT, &act, NULL) == -1)
    {
        log_error("%d: %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void exit_handler(int sig, siginfo_t *sig_info, void *context)
{
    UNUSED(sig);
    UNUSED(sig_info);
    UNUSED(context);
    
    got_exit = true;
}
