/*
 * Dynamic IPv4 Link-Local Address Configuration
 *
 * This implements the main n-ipv4ll API. It is built around n-acd, with
 * analogus limetime rules. First, the parameters must be set by the caller,
 * then the engine is started on demand, and stopped if no longer needed. While
 * stopped, parameters may be changed for a next run. During the entire lifetime
 * the context can be dispatched. That is, the dispatcher does not have to be
 * aware of the context state.
 *
 * If a conflict is detected, the IPv4LL engine reports to the caller and stops
 * the engine. The caller can now modify parameters and restart the engine, if
 * required.
 */

#include <errno.h>
#include <netinet/in.h>
#include <n-acd.h>
#include <stdlib.h>
#include "n-ipv4ll.h"

#define _public_ __attribute__((__visibility__("default")))

#define IPV4LL_NETWORK UINT32_C(0xa9fe0000)

struct NIpv4ll {
        /* context */
        unsigned long n_refs;
        struct drand48_data state;

        /* runtime */
        NAcd *acd;
        NIpv4llFn fn;
        void *userdata;
        uint16_t n_iteration;
};

_public_ int n_ipv4ll_new(NIpv4ll **llp) {
        NIpv4ll *ll;
        int r;

        ll = calloc(1, sizeof(*ll));
        if (!ll)
                return -ENOMEM;

        ll->n_refs = 1;

        r = n_acd_new(&ll->acd);
        if (r < 0)
                goto error;

        *llp = ll;
        return 0;

error:
        n_ipv4ll_unref(ll);
        return r;
}

_public_ NIpv4ll *n_ipv4ll_ref(NIpv4ll *ll) {
        if (ll)
                ++ll->n_refs;
        return ll;
}

_public_ NIpv4ll *n_ipv4ll_unref(NIpv4ll *ll) {
        if (!ll || --ll->n_refs)
                return NULL;

        n_ipv4ll_stop(ll);

        n_acd_unref(ll->acd);

        free(ll);

        return NULL;
}

_public_ bool n_ipv4ll_is_running(NIpv4ll *ll) {
        return n_acd_is_running(ll->acd);
}

_public_ void n_ipv4ll_get_fd(NIpv4ll *ll, int *fdp) {
        n_acd_get_fd(ll->acd, fdp);
}

_public_ void n_ipv4ll_get_ifindex(NIpv4ll *ll, int *ifindexp) {
        n_acd_get_ifindex(ll->acd, ifindexp);
}

_public_ void n_ipv4ll_get_mac(NIpv4ll *ll, struct ether_addr *macp) {
        n_acd_get_mac(ll->acd, macp);
}

_public_ int n_ipv4ll_get_ip(NIpv4ll *ll, struct in_addr *ipp) {
        struct in_addr ip;

        n_acd_get_ip(ll->acd, &ip);

        if (ip.s_addr == INADDR_ANY)
                return -EADDRNOTAVAIL;

        *ipp = ip;

        return 0;
}

_public_ int n_ipv4ll_set_ifindex(NIpv4ll *ll, int ifindex) {
        return n_acd_set_ifindex(ll->acd, ifindex);
}

_public_ int n_ipv4ll_set_mac(NIpv4ll *ll, const struct ether_addr *mac) {
        return n_acd_set_mac(ll->acd, mac);
}

_public_ int n_ipv4ll_dispatch(NIpv4ll *ll) {
        int r;

        n_ipv4ll_ref(ll);
        r = n_acd_dispatch(ll->acd);
        n_ipv4ll_unref(ll);

        return r;
}

_public_ int n_ipv4ll_announce(NIpv4ll *ll) {
        return n_acd_announce(ll->acd, N_ACD_DEFEND_ONCE);
}

static void n_ipv4ll_select_ip(NIpv4ll *ll, struct in_addr *ip) {
        for (;;) {
                long int result;
                uint16_t offset;

                (void) mrand48_r(&ll->state, &result);

                offset = result ^ (result >> 16);

                /*
                 * The first and the last 256 addresses in the subnet are
                 * reserved.
                 */
                if (offset < 0x100 || offset > 0xfdff)
                        continue;

                ip->s_addr = htobe32(IPV4LL_NETWORK | offset);
                break;
        }
}

_public_ int n_ipv4ll_set_enumeration(NIpv4ll *ll, uint64_t enumeration) {
        if (n_ipv4ll_is_running(ll))
                return -EBUSY;

        (void) seed48_r((unsigned short int*) &enumeration, &ll->state);

        return 0;
}

static void n_ipv4ll_handle_acd(NAcd *acd, void *userdata, unsigned int event, const struct ether_arp *conflict) {
        NIpv4ll *ll = userdata;
        struct in_addr ip;
        int r;

        switch (event) {
        case N_ACD_EVENT_READY:
                ll->fn(ll, ll->userdata, N_IPV4LL_EVENT_READY, NULL);
                break;

        case N_ACD_EVENT_DEFENDED:
                ll->fn(ll, ll->userdata, N_IPV4LL_EVENT_DEFENDED, conflict);
                break;

        case N_ACD_EVENT_CONFLICT:
                ll->fn(ll, ll->userdata, N_IPV4LL_EVENT_CONFLICT, conflict);

                /* fall-through */
        case N_ACD_EVENT_USED:
                ll->n_iteration ++;

                n_acd_stop(ll->acd);
                n_ipv4ll_select_ip(ll, &ip);
                n_acd_set_ip(ll->acd, &ip);
                r = n_acd_start(ll->acd, n_ipv4ll_handle_acd, ll);
                if (r >= 0)
                        return;

                /*
                 * Failed to restart ACD. Give up and report the
                 * failure to the caller. Fall-through.
                 */
        case N_ACD_EVENT_DOWN:
                ll->fn(ll, ll->userdata, N_IPV4LL_EVENT_DOWN, NULL);
                break;
        }
}

_public_ int n_ipv4ll_start(NIpv4ll *ll, NIpv4llFn fn, void *userdata) {
        struct in_addr ip;
        int r;

        if (!fn)
                return -EINVAL;

        ll->fn = fn;
        ll->userdata = userdata;

        n_ipv4ll_select_ip(ll, &ip);

        r = n_acd_set_ip(ll->acd, &ip);
        if (r < 0)
                goto error;

        r = n_acd_start(ll->acd, n_ipv4ll_handle_acd, ll);
        if (r < 0)
                goto error;

        return 0;

error:
        n_ipv4ll_stop(ll);
        return r;
}

_public_ void n_ipv4ll_stop(NIpv4ll *ll) {
        ll->userdata = NULL;
        ll->fn = NULL;
        n_acd_stop(ll->acd);
}
