SUBDIRS = src data docs testsuite

EXTRA_DIST = AUTHORS \
             COPYING \
             HACKING \
             NEWS \
             README \
             THANKS \
             TODO \
             autogen.sh \
             waf \
             wscript

DISTCLEANFILES = config.status \
                 config.log

DISTCHECK_CONFIGURE_FLAGS = --enable-introspection --enable-gtk-doc

ACLOCAL_AMFLAGS = -I build/m4
