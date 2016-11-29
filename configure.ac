# ------------------------------------------------------------------------------
# versions

AC_PREREQ([2.64])
AC_INIT([n-ipv4ll],
        [1],
        [http://www.github.com/nettools/n-ipv4ll/issues],
        [n-ipv4ll],
        [http://www.github.com/nettools/n-ipv4ll])

# ------------------------------------------------------------------------------
# autotools

AC_CONFIG_SRCDIR([src/n-ipv4ll.h])
AC_CONFIG_MACRO_DIR([build/m4])
AC_CONFIG_HEADERS([build/config.h])
AC_CONFIG_AUX_DIR([build/build-aux])

# suppress conflicting defaults
CFLAGS=${CFLAGS:-""}
CXXFLAGS=${CXXFLAGS:-""}

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AM_MAINTAINER_MODE([enable])
AM_INIT_AUTOMAKE([foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects parallel-tests])
AM_SILENT_RULES([yes])
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([CANONICAL_HOST], "$host", [Canonical host string.])
AC_PROG_CC_C99
AC_PROG_RANLIB
AC_PROG_SED
AC_PROG_LN_S

m4_pattern_forbid([^_?PKG_[A-Z_]+$],[*** pkg.m4 missing, please install pkg-config])

# ------------------------------------------------------------------------------
have_nacd=no
PKG_CHECK_MODULES(NACD, [ libnacd ],
        [AC_DEFINE(HAVE_NACD, 1, [Define if n-acd is available]) have_nacd=yes], have_nacd=no)
if test "x$have_nacd" = xno; then
        AC_MSG_ERROR([*** n-acd support required but libraries not found])
fi
AM_CONDITIONAL(HAVE_NACD, [test "$have_nacd" = "yes"])

# ------------------------------------------------------------------------------
# report

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE_NAME $VERSION

        n-acd:                  ${have_nacd}

        prefix:                 ${prefix}
        exec_prefix:            ${exec_prefix}
        includedir:             ${includedir}
        libdir:                 ${libdir}

        CFLAGS:                 ${CFLAGS}
        CPPFLAGS:               ${CPPFLAGS}
        LDFLAGS:                ${LDFLAGS}
])
