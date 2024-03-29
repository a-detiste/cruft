# Makefile for cruft

CFLAGS += -Wall -W -pedantic-errors

INS = install

DATADIRS = explain filters-broken_symlinks filters-unex

DESTDIR = `pwd`/debian/tmp

PROGS1 = canonical filter_shell merge_diff readlinks
PROGS2 = dash-search
PROGS  = $(PROGS1) $(PROGS2)

all: $(PROGS)

install1: $(PROGS1) filter filters_list cruft_find filter_ignores check_type_symlink z_cruft
	mkdir -p $(DESTDIR)/usr/lib/cruft/
	$(INS) $^ $(DESTDIR)/usr/lib/cruft/

install11: common.sh common_legacy.sh common.pl
	mkdir -p $(DESTDIR)/usr/lib/cruft/
	$(INS) -D -m 0644 $^ $(DESTDIR)/usr/lib/cruft/

install2:  $(PROGS2)
	$(INS) -D $< $(DESTDIR)/usr/bin/$<

installdirs: $(DATADIRS)
	set -e ; for d in $^ ; do \
		mkdir -p $(DESTDIR)/usr/lib/cruft/$$d; \
		find $$d -maxdepth 1 -type f | xargs -i cp '{}' $(DESTDIR)/usr/lib/cruft/$$d/; \
	done

install: install1 install11 install2 installdirs cruft
	$(INS) -D cruft $(DESTDIR)/usr/sbin/cruft

clean:
	rm -f $(PROGS) merge_diff.o merge_diff_funcs.o shellexp.o *~ debian/*~ \
	 shellexptest.o shellexptest merge_diff_test.o merge_diff_test \
	 merge_diff_common.o river.o fn_stream.o
	rm -rf merge_diff_tests/*/run
	rm -f design.png

check: shellexptest merge_diff_test
	./test-shellexp
	cd merge_diff_tests && ./test
	echo Checking for trailing whitespaces
	grep -E -R -H " +$$" filters-unex/ || true
	! grep -E -R -q " +$$" filters-unex/
	echo Checking for trailing slashes
	grep -E -R -H "/+$$" filters-unex/ || true
	! grep -E -R -q "/+$$" filters-unex/

shellexptest: shellexptest.o shellexp.o
filter_shell dash-search: shellexp.o
merge_diff_test: merge_diff_test.o merge_diff_funcs.o merge_diff_common.o river.o fn_stream.o
merge_diff:      merge_diff.o      merge_diff_funcs.o merge_diff_common.o river.o fn_stream.o
merge_diff_funcs.o merge_diff_test.o merge_diff_common.o river.o fn_stream.o merge_diff.o: merge_diff_common.h

design.png: design.dia
	unset DISPLAY ; dia --size 800x740 --export tmp-$@ $<
	optipng -o 7 tmp-$@
	mv tmp-$@ $@

.PHONY: all clean install check
