#
# Top-level Makefile for SRILM
#
# $Header: /home/srilm/devel/RCS/Makefile,v 1.51 2009/12/02 20:39:16 stolcke Exp $
#

SRILM = /home/jnfem112/DSP/HW3/srilm-1.5.10
MACHINE_TYPE := $(shell $(SRILM)/sbin/machine-type)

RELEASE := $(shell cat RELEASE)

INFO = \
	CHANGES \
	RELEASE \
	README \
	doc \
	Copyright \
	License

MODULES = \
	misc \
	dstruct \
	lm \
	flm \
	lattice \
	utils

EXCLUDE = \
	me \
	contrib \
	lm/src/test \
	flm/src/test \
	lattice/src/test \
	dstruct/src/test \
	utils/src/fsmtest \
	common/COMPILE-HOSTS \
	misc/src/SRILMversion.h \
	EXCLUDE \
	DONE

MAKE_VARS = \
	SRILM=$(SRILM) \
	MACHINE_TYPE=$(MACHINE_TYPE) \
	OPTION=$(OPTION) \
	MAKE_PIC=$(MAKE_PIC)

World:	dirs
	$(MAKE) init
	$(MAKE) release-headers
	$(MAKE) depend
	$(MAKE) release-libraries
	$(MAKE) release-programs
	$(MAKE) release-scripts

depend-all:	dirs release-headers
	@gawk '!/^#/ { print $$1, $$2, $$3 }' common/COMPILE-HOSTS | sort -u | \
	while read prog host type; do \
		rm -f DONE; (set -x; \
		$$prog $$host "cd $(SRILM); $(MAKE) $(MFLAGS) MACHINE_TYPE=$$type OPTION=$$option init depend && touch DONE" < /dev/null); \
		[ -f DONE ] || exit 1; \
	done; rm -f DONE

compile-all:	dirs
	@gawk '!/^#/' common/COMPILE-HOSTS | \
	while read prog host type option; do \
		rm -f DONE; (set -x; \
		$$prog $$host "cd $(SRILM); $(MAKE) $(MFLAGS) MACHINE_TYPE=$$type OPTION=$$option init release-libraries release-programs && touch DONE" < /dev/null); \
		[ -f DONE ] || exit 1; \
	done; rm -f DONE
	

dirs:
	-mkdir include lib bin

init depend all programs release clean cleaner cleanest sanitize desanitize \
release-headers release-libraries release-programs release-scripts:
	for subdir in $(MODULES); do \
		(cd $$subdir/src; $(MAKE) $(MAKE_VARS) $@) || exit 1; \
	done

test try:
	for subdir in $(MODULES); do \
		[ ! -d $$subdir/test ] || \
		(cd $$subdir/test; $(MAKE) $(MAKE_VARS) $@) || exit 1; \
	done

# files needed for the web page
WWW_DOCS = CHANGES License INSTALL RELEASE
WWW_DIR = /home/spftp/www/DocumentRoot/projects/srilm

www:	$(WWW_DOCS)
	ginstall -m 444 $(WWW_DOCS) $(WWW_DIR)/docs
	ginstall -m 444 man/html/* $(WWW_DIR)/manpages

TAR = /usr/local/gnu/bin/tar

package:	EXCLUDE
	$(TAR) cvzXf EXCLUDE ../srilm-$(RELEASE).tar.gz .

package_notest:	EXCLUDE
	echo test >> EXCLUDE
	$(TAR) cvzXf EXCLUDE ../srilm-$(RELEASE)-notest.tar.gz .
	rm EXCLUDE

package_bin:	EXCLUDE-$(MACHINE_TYPE)
	$(TAR) cvzXf EXCLUDE-$(MACHINE_TYPE) ../srilm-$(RELEASE)-$(MACHINE_TYPE).tar.gz $(INFO) include lib man bin sbin
	rm -f EXCLUDE-$(MACHINE_TYPE)

package_x:
	$(MAKE) $(MAKE_VARS) sanitize
	$(MAKE) $(MAKE_VARS) RELEASE=$(RELEASE)_x package
	$(MAKE) $(MAKE_VARS) desanitize

EXCLUDE:	force
	(find bin/* lib/* */bin/* */obj/* */src/test */test/output */test/logs -type d -print -prune ; \
	find $(EXCLUDE) include bin -print; \
	find . \( -name Makefile.site.\* -o -name "*.~[0-9]*" -o -name Dependencies.\* -o -name core -o -name "core.[0-9]*" -o -name \*.3rdparty -o -name .gdb_history -o -name out.\* -o -name "*[._]pure[._]*" -o -type l -o -name RCS \) -print) | \
	sed 's,^\./,,' > $@

EXCLUDE-$(MACHINE_TYPE):	EXCLUDE
	egrep -v '/$(MACHINE_TYPE)[^~]*$$' EXCLUDE > $@
	egrep '$(MACHINE_TYPE).*[._]pure[._]' EXCLUDE >> $@
	egrep '$(MACHINE_TYPE)_[gp]' EXCLUDE >> $@

force:

