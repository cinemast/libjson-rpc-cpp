
all: debian8 debian9 debian10 ubuntu1604 ubuntu1704 arch fedora

debian8: Debian8.Dockerfile
	docker build --rm -f $< ..

debian9: Debian9.Dockerfile
	docker build --rm -f $< ..

debian10: Debian10.Dockerfile
	docker build --rm -f $< ..

ubuntu1604: Ubuntu1604.Dockerfile
	docker build --rm -f $< ..

ubuntu1704: Ubuntu1704.Dockerfile
	docker build --rm -f $< ..

arch: ArchLinux.Dockerfile
	docker build --rm -f $< ..

fedora: Fedora.Dockerfile
	docker build --rm -f $< ..
