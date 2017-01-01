/*
 * Tests for n-ipv4ll API
 * This verifies the visibility and availability of the public API of the
 * n-ipv4ll library.
 */

#include <stdlib.h>
#include "test.h"

static void test_api_constants(void) {
        assert(N_IPV4LL_EVENT_READY != _N_IPV4LL_EVENT_N);
        assert(N_IPV4LL_EVENT_DEFENDED != _N_IPV4LL_EVENT_N);
        assert(N_IPV4LL_EVENT_CONFLICT != _N_IPV4LL_EVENT_N);
        assert(N_IPV4LL_EVENT_DOWN != _N_IPV4LL_EVENT_N);
}

static void test_api_management(void) {
        NIpv4ll *ipv4ll = NULL;
        int r;

        /* new/ref/unref/unrefp */

        n_ipv4ll_unrefp(&ipv4ll);

        r = n_ipv4ll_new(&ipv4ll);
        assert(r >= 0);
        n_ipv4ll_ref(ipv4ll);
        n_ipv4ll_unref(ipv4ll);


        n_ipv4ll_unref(ipv4ll);
}

static void test_api_configuration(void) {
        struct ether_addr mac;
        struct in_addr ip;
        NIpv4ll *ipv4ll;
        int r, ifindex;

        /* {get,set}_{ifindex,mac,enumeration} */

        r = n_ipv4ll_new(&ipv4ll);
        assert(r >= 0);

        r = n_ipv4ll_set_ifindex(ipv4ll, 1);
        assert(r >= 0);
        r = n_ipv4ll_set_mac(ipv4ll, &(struct ether_addr){ { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54 } });
        assert(r >= 0);
        r = n_ipv4ll_set_enumeration(ipv4ll, 0);
        assert(r >= 0);

        n_ipv4ll_get_ifindex(ipv4ll, &ifindex);
        assert(ifindex == 1);
        n_ipv4ll_get_mac(ipv4ll, &mac);
        assert(!memcmp(mac.ether_addr_octet, (uint8_t[ETH_ALEN]){ 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54 }, ETH_ALEN));
        r = n_ipv4ll_get_ip(ipv4ll, &ip);
        assert(r == -EADDRNOTAVAIL);

        n_ipv4ll_unref(ipv4ll);
}

static void test_api_runtime(void) {
        NIpv4llFn fn = NULL;
        NIpv4ll *ipv4ll;
        int r;

        /* get_fd/is_running/dispatch/start/stop/announce */

        r = n_ipv4ll_new(&ipv4ll);
        assert(r >= 0);

        n_ipv4ll_get_fd(ipv4ll, &r);
        assert(r >= 0);
        r = n_ipv4ll_is_running(ipv4ll);
        assert(!r);
        r = n_ipv4ll_dispatch(ipv4ll);
        assert(r >= 0);
        r = n_ipv4ll_start(ipv4ll, fn, NULL);
        assert(r < 0);
        n_ipv4ll_stop(ipv4ll);
        r = n_ipv4ll_announce(ipv4ll);
        assert(r < 0);

        n_ipv4ll_unref(ipv4ll);
}

int main(int argc, char **argv) {
        test_api_constants();
        test_api_management();
        test_api_configuration();
        test_api_runtime();
        return 0;
}
