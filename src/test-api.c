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

        /* new/free/freep */

        n_ipv4ll_freep(&ipv4ll);

        r = n_ipv4ll_new(&ipv4ll);
        assert(r >= 0);

        n_ipv4ll_free(ipv4ll);
}

static void test_api_runtime(void) {
        NIpv4llConfig config = {
                .ifindex = 1,
                .mac = { { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54 } },
        };
        NIpv4ll *ipv4ll;
        int r;

        /* get_fd/dispatch/pop_event/start/stop/announce */

        r = n_ipv4ll_new(&ipv4ll);
        assert(r >= 0);

        n_ipv4ll_get_fd(ipv4ll, &r);
        assert(r >= 0);
        r = n_ipv4ll_dispatch(ipv4ll);
        assert(r >= 0);
        r = n_ipv4ll_pop_event(ipv4ll, NULL);
        assert(r == -EAGAIN);
        r = n_ipv4ll_start(ipv4ll, &config);
        assert(r >= 0);
        n_ipv4ll_stop(ipv4ll);
        r = n_ipv4ll_announce(ipv4ll);
        assert(r < 0);

        n_ipv4ll_free(ipv4ll);
}

int main(int argc, char **argv) {
        test_api_constants();
        test_api_management();
        test_api_runtime();
        return 0;
}
