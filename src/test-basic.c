/*
 * Test with two conflicts
 * Run the IPv4LL engine when the two first attempts will fail, and the third
 * one succeed. This should just pass through, with a short, random timeout.
 */

#include <stdlib.h>
#include "test.h"

static void test_basic(int ifindex, const struct ether_addr *mac) {
        NIpv4llConfig config = {
                .ifindex = ifindex,
                .mac = *mac,
        };
        struct pollfd pfds;
        NIpv4ll *acd;
        int r, fd;

        r = n_ipv4ll_new(&acd);
        assert(r >= 0);

        n_ipv4ll_get_fd(acd, &fd);
        r = n_ipv4ll_start(acd, &config);
        assert(r >= 0);

        for (;;) {
                NIpv4llEvent event;
                pfds = (struct pollfd){ .fd = fd, .events = POLLIN };
                r = poll(&pfds, 1, -1);
                assert(r >= 0);

                r = n_ipv4ll_dispatch(acd);
                assert(r >= 0);

                r = n_ipv4ll_pop_event(acd, &event);
                if (r >= 0) {
                        assert(event.event == N_IPV4LL_EVENT_READY);
                        assert(event.ready.address.s_addr == htobe32((169 << 24) | (254 << 16) | (148 << 8) | 109));

                        break;
                } else {
                        assert(r == -EAGAIN);
                }
        }

        n_ipv4ll_free(acd);
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
