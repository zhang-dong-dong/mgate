
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src modules po
dist_doc_DATA = README 
EXTRA_DIST = @PACKAGE_NAME@.spec.in @PACKAGE_NAME@.cfg.in.in @PACKAGE_NAME@.service.in.in

DISTCLEANFILES = @PACKAGE_NAME@.spec @PACKAGE_NAME@.cfg @PACKAGE_NAME@.cfg.in

spec= @PACKAGE_NAME@.spec

systemdsystemunitdir=@systemdsystemunitdir@

rpm: dist @PACKAGE_NAME@.spec
	rpmbuild  --nodeps -bb \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_specdir `pwd`" \
			--define "_rpmdir `pwd`" \
			--define "_srcrpmdir `pwd`" \
			$(spec) && echo rpm is at `pwd`/`arch`/@PACKAGE_NAME@-@VERSION@-1.`arch`.rpm


srpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bs \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_srcrpmdir `pwd`" \
		    --define "_rpmdir `pwd`" \
			--define "_specdir `pwd`" \
			$(spec)

sysconfigdir = @sysconfdir@

sysconfig_DATA = @PACKAGE_NAME@.cfg

@PACKAGE_NAME@.cfg:@PACKAGE_NAME@.cfg.in
		( \
		prefix=${prefix}; \
		localstatedir=${localstatedir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	   ) > $@	

if HAVE_SYSTEMD
systemdsystemunit_DATA = @PACKAGE_NAME@.service

@PACKAGE_NAME@.service:@PACKAGE_NAME@.service.in
		( \
		exec_prefix=${exec_prefix}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	   ) > $@	

CLEANFILES = 	@PACKAGE_NAME@.service
endif

#===============================for test===========

test : $(top_builddir)/src/mgate /tmp/${prefix}/share/locale/
	@	$(top_builddir)/src/mgate --module_dir=$(top_builddir)/modules  --config=$(top_srcdir)/mgate.cfg --locale=/tmp/${prefix}/share/locale/ --http-root=.  ${ARG}

/tmp/${prefix}/share/locale/ : po/zh_CN.gmo
	@ ( cd po && $(MAKE) DESTDIR=/tmp install ) > /dev/null 2> /dev/null

po/zh_CN.gmo: po/zh_CN.po
	cd po && $(MAKE) zh_CN.gmo
	
$(top_builddir)/src/mgate :
	cd src && $(MAKE)

gdb : $(top_builddir)/src/mgate /tmp/${prefix}/share/locale/
	gdbserver localhost:5000 $(top_builddir)/src/mgate --module_dir=$(top_builddir)/modules  --config=$(top_srcdir)/mgate.cfg --locale=/tmp/${prefix}/share/locale/  ${ARG}
