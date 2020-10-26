ROOT=Debian

deb:
	cd src; make
	install -m 755 src/kanon-snow $(ROOT)/usr/bin
	cp -r src/etc $(ROOT)
	chmod 755 $(ROOT)/etc $(ROOT)/etc/kanon-snow
	chmod 644 $(ROOT)/etc/kanon-snow/*
	fakeroot dpkg-deb --build $(ROOT) .
clean:
	rm -fr $(ROOT)/etc $(ROOT)/usr/bin/*
	rm -f kanon-snow_*.*-*_*.deb
