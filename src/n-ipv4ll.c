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
        struct drand48_data state;

        /* runtime */
        NAcd *acd;
        NAcdConfig config;

        /* pending event */
        NIpv4llEvent event;
};

_public_ int n_ipv4ll_new(NIpv4ll **llp) {
        NIpv4ll *ll;
        int r;

        ll = calloc(1, sizeof(*ll));
        if (!ll)
                return -ENOMEM;

        r = n_acd_new(&ll->acd);
        if (r < 0)
                goto error;

        ll->event.event = _N_IPV4LL_EVENT_INVALID;

        *llp = ll;
        return 0;

error:
        n_ipv4ll_free(ll);
        return r;
}

_public_ NIpv4ll *n_ipv4ll_free(NIpv4ll *ll) {
        if (!ll)
                return NULL;

        n_ipv4ll_stop(ll);

        n_acd_free(ll->acd);

        free(ll);

        return NULL;
}

_public_ void n_ipv4ll_get_fd(NIpv4ll *ll, int *fdp) {
        n_acd_get_fd(ll->acd, fdp);
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

static void n_ipv4ll_handle_acd_event(NIpv4ll *ll, NAcdEvent *event) {
        int r;

        switch (event->event) {
        case N_ACD_EVENT_READY:
                ll->event.event = N_IPV4LL_EVENT_READY;
                ll->event.ready.address = ll->config.ip;
                break;

        case N_ACD_EVENT_DEFENDED:
                ll->event.event = N_IPV4LL_EVENT_DEFENDED;
                ll->event.defended.packet = event->defended.packet;
                break;

        case N_ACD_EVENT_CONFLICT:
                ll->event.event = N_IPV4LL_EVENT_CONFLICT;
                ll->event.conflict.packet = event->conflict.packet;

                /* fall-through */
        case N_ACD_EVENT_USED:
                n_acd_stop(ll->acd);
                n_ipv4ll_select_ip(ll, &ll->config.ip);
                r = n_acd_start(ll->acd, &ll->config);
                if (r >= 0)
                        return;

                /*
                 * Failed to restart ACD. Give up and report the
                 * failure to the caller.
                 */

                /* fall-through */
        case N_ACD_EVENT_DOWN:
                ll->event.event = N_IPV4LL_EVENT_DOWN;
                break;
        }
}

_public_ int n_ipv4ll_dispatch(NIpv4ll *ll) {
        int r;

        r =  n_acd_dispatch(ll->acd);
        if (r < 0)
                return r;

        for (;;) {
                NAcdEvent event;

                r = n_acd_pop_event(ll->acd, &event);
                if (r == -EAGAIN)
                        break;
                else if (r < 0)
                        return r;

                n_ipv4ll_handle_acd_event(ll, &event);
        }

        return 0;
}

_public_ int n_ipv4ll_pop_event(NIpv4ll *ll, NIpv4llEvent *eventp) {
        if (ll->event.event == _N_IPV4LL_EVENT_INVALID)
                return -EAGAIN;

        *eventp = ll->event;
        ll->event.event = _N_IPV4LL_EVENT_INVALID;

        return 0;
}

_public_ int n_ipv4ll_announce(NIpv4ll *ll) {
        return n_acd_announce(ll->acd, N_ACD_DEFEND_ONCE);
}

_public_ int n_ipv4ll_start(NIpv4ll *ll, NIpv4llConfig *config) {
        NAcdConfig acd_config = {
                .ifindex = config->ifindex,
                .mac = config->mac,
        };
        int r;

        (void) seed48_r((unsigned short int*) &config->enumeration, &ll->state);
        n_ipv4ll_select_ip(ll, &acd_config.ip);

        r = n_acd_start(ll->acd, &acd_config);
        if (r < 0)
                return r;

        ll->config = acd_config;

        return 0;
}

_public_ void n_ipv4ll_stop(NIpv4ll *ll) {
        n_acd_stop(ll->acd);
        ll->event.event = _N_IPV4LL_EVENT_INVALID;
}
