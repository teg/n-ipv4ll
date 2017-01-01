#pragma once

/*
 * IPv4 Link Local Address Configuration
 *
 * This is the public header of the n-ipv4ll library, implementing Dynamic IPv4
 * Link-Local Address Configuration as described in RFC-3927. This header
 * defines the public API and all entry points of n-ipv4ll.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct ether_addr;
struct ether_arp;
struct in_addr;

typedef struct NIpv4ll NIpv4ll;
typedef void (*NIpv4llFn) (NIpv4ll *acd, void *userdata, unsigned int event, const struct ether_arp *conflict);

enum {
        N_IPV4LL_EVENT_READY,
        N_IPV4LL_EVENT_DEFENDED,
        N_IPV4LL_EVENT_CONFLICT,
        N_IPV4LL_EVENT_DOWN,
        _N_IPV4LL_EVENT_N,
};

int n_ipv4ll_new(NIpv4ll **acdp);
NIpv4ll *n_ipv4ll_ref(NIpv4ll *acd);
NIpv4ll *n_ipv4ll_unref(NIpv4ll *acd);

bool n_ipv4ll_is_running(NIpv4ll *acd);
void n_ipv4ll_get_fd(NIpv4ll *acd, int *fdp);
void n_ipv4ll_get_ifindex(NIpv4ll *acd, int *ifindexp);
void n_ipv4ll_get_mac(NIpv4ll *acd, struct ether_addr *macp);
int n_ipv4ll_get_ip(NIpv4ll *acd, struct in_addr *ipp);

int n_ipv4ll_set_ifindex(NIpv4ll *acd, int ifindex);
int n_ipv4ll_set_mac(NIpv4ll *acd, const struct ether_addr *mac);
int n_ipv4ll_set_enumeration(NIpv4ll *acd, uint64_t enumeration);

int n_ipv4ll_dispatch(NIpv4ll *acd);
int n_ipv4ll_start(NIpv4ll *acd, NIpv4llFn fn, void *userdata);
void n_ipv4ll_stop(NIpv4ll *acd);
int n_ipv4ll_announce(NIpv4ll *acd);

static inline void n_ipv4ll_unrefp(NIpv4ll **acd) {
        if (*acd)
                n_ipv4ll_unref(*acd);
}

#ifdef __cplusplus
}
#endif
