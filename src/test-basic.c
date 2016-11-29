/*
 * Test with two conflicts
 * Run the IPv4LL engine when the two first attempts will fail, and the third
 * one succeed. This should just pass through, with a short, random timeout.
 */

#include <stdlib.h>
#include "test.h"

static void test_basic_fn(NIpv4ll *ll, void *userdata, unsigned int event, const struct ether_arp *conflict) {
        struct in_addr ip;
        bool *running = userdata;

        n_ipv4ll_get_ip(ll, &ip);
        assert(ip.s_addr == htobe32((169 << 24) | (254 << 16) | (1 << 8) | 2));

        assert(event == N_IPV4LL_EVENT_READY);
        *running = false;
}

static void test_basic(int ifindex, const struct ether_addr *mac) {
        struct pollfd pfds;
        bool running;
        NIpv4ll *acd;
        int r, fd;

        r = n_ipv4ll_new(&acd);
        assert(r >= 0);

        r = n_ipv4ll_set_ifindex(acd, ifindex);
        assert(r >= 0);
        r = n_ipv4ll_set_mac(acd, mac);
        assert(r >= 0);

        n_ipv4ll_get_fd(acd, &fd);
        r = n_ipv4ll_start(acd, test_basic_fn, &running);
        assert(r >= 0);

        for (running = true; running; ) {
                pfds = (struct pollfd){ .fd = fd, .events = POLLIN };
                r = poll(&pfds, 1, -1);
                assert(r >= 0);

                r = n_ipv4ll_dispatch(acd);
                assert(r >= 0);
        }

        n_ipv4ll_unref(acd);
}

int main(int argc, char **argv) {
        struct ether_addr mac;
        int r, ifindex;

        r = test_setup();
        if (r)
                return r;

        test_veth_new(&ifindex, &mac, NULL, NULL);
        test_basic(ifindex, &mac);

        return 0;
}
