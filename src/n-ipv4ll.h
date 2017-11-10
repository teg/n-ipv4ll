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

#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <stdbool.h>

typedef struct NIpv4ll NIpv4ll;

typedef struct NIpv4llConfig {
        unsigned int ifindex;
        struct ether_addr mac;
        uint64_t enumeration;
} NIpv4llConfig;

typedef struct NIpv4llEvent {
        unsigned int event;
        union {
                struct {
                        struct in_addr address;
                } ready;
                struct {
                        struct ether_arp packet;
                } defended, conflict;
                struct {
                } down;
        };
} NIpv4llEvent;

enum {
        N_IPV4LL_EVENT_READY,
        N_IPV4LL_EVENT_DEFENDED,
        N_IPV4LL_EVENT_CONFLICT,
        N_IPV4LL_EVENT_DOWN,
        _N_IPV4LL_EVENT_N,
        _N_IPV4LL_EVENT_INVALID,
};

int n_ipv4ll_new(NIpv4ll **llp);
NIpv4ll *n_ipv4ll_free(NIpv4ll *ll);

void n_ipv4ll_get_fd(NIpv4ll *ll, int *fdp);

int n_ipv4ll_dispatch(NIpv4ll *ll);
int n_ipv4ll_pop_event(NIpv4ll *ll, NIpv4llEvent *eventp);
int n_ipv4ll_announce(NIpv4ll *ll);

int n_ipv4ll_start(NIpv4ll *ll, NIpv4llConfig *config);
void n_ipv4ll_stop(NIpv4ll *ll);

static inline void n_ipv4ll_freep(NIpv4ll **llp) {
        if (*llp)
                n_ipv4ll_free(*llp);
}

#ifdef __cplusplus
}
#endif
