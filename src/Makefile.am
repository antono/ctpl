
bin_PROGRAMS    = 
check_PROGRAMS  = test
lib_LTLIBRARIES = libctpl.la

libctpl_la_CPPFLAGS = -DG_LOG_DOMAIN=\"CTPL\" -DCTPL_COMPILATION
libctpl_la_CFLAGS   = @GLIB_CFLAGS@ @GIO_CFLAGS@
libctpl_la_LDFLAGS  = -version-info @CTPL_LTVERSION@
libctpl_la_LIBADD   = @GLIB_LIBS@ @GIO_LIBS@ -lm
libctpl_la_SOURCES  = ctpl-environ.c \
                      ctpl-eval.c \
                      ctpl-io.c \
                      ctpl-input-stream.c \
                      ctpl-lexer.c \
                      ctpl-lexer-expr.c \
                      ctpl-mathutils.c \
                      ctpl-output-stream.c \
                      ctpl-parser.c \
                      ctpl-stack.c \
                      ctpl-token.c \
                      ctpl-value.c \
                      ctpl-version.c

ctplincludedir = $(includedir)/ctpl
ctplinclude_HEADERS = ctpl.h \
                      ctpl-environ.h \
                      ctpl-eval.h \
                      ctpl-io.h \
                      ctpl-input-stream.h \
                      ctpl-lexer.h \
                      ctpl-lexer-expr.h \
                      ctpl-output-stream.h \
                      ctpl-parser.h \
                      ctpl-token.h \
                      ctpl-value.h \
                      ctpl-version.h

EXTRA_DIST          = ctpl-lexer-private.h \
                      ctpl-mathutils.h \
                      ctpl-stack.h \
                      ctpl-token-private.h

if BUILD_CTPL
bin_PROGRAMS += ctpl

ctpl_CFLAGS   = @GLIB_CFLAGS@ @GIO_CFLAGS@ @GIO_UNIX_CFLAGS@
ctpl_LDADD    = libctpl.la @GLIB_LIBS@ @GIO_LIBS@ @GIO_UNIX_LIBS@
ctpl_SOURCES  = ctpl.c
endif

# test may use internal symbols (debuging and friends), so don't use the real
# library but only the object files
test_CPPFLAGS = $(libctpl_la_CPPFLAGS)
test_CFLAGS   = $(libctpl_la_CFLAGS)
test_LDADD    = $(libctpl_la_LIBADD)
test_SOURCES  = test.c \
                $(libctpl_la_SOURCES)

CLEANFILES    =

# Gobject introspection support
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION

introspection_sources = $(libctpl_la_SOURCES) $(ctplinclude_HEADERS)

Ctpl_1.0.gir: $(lib_LTLIBRARIES)
Ctpl_1_0_gir_FILES = $(introspection_sources)
Ctpl_1_0_gir_LIBS = $(lib_LTLIBRARIES)
Ctpl_1_0_gir_INCLUDES = GLib-2.0
Ctpl_1_0_gir_EXPORT_PACKAGES = ctpl
Ctpl_1_0_gir_CFLAGS = $(INCLUDES) -DCTPL_COMPILATION
Ctpl_1_0_gir_SCANNERFLAGS = --identifier-prefix=ctpl --c-include "ctpl/ctpl.h" --warn-all
INTROSPECTION_GIRS += Ctpl-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif
