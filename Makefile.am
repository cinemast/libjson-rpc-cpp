EXTRA_DIST = libjsonrpccpp.spec

SUBDIRS = \
	. \
	src
	
rpm: dist
	mkdir -p `pwd`/RPMBUILD/{BUILD,SOURCES,RPMS,SRPMS,SPECS}; \
	rpmbuild -ta --define "%_topdir `pwd`/RPMBUILD" libjsonrpccp*.gz
