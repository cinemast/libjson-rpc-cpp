
all: debian8 debian9 ubuntu1604 ubuntu1704 arch fedora

debian8: Debian8.Dockerfile
	docker build -f $< ..

debian9: Debian9.Dockerfile
	docker build -f $< ..

ubuntu1604: Ubuntu1604.Dockerfile
	docker build -f $< ..

ubuntu1704: Ubuntu1704.Dockerfile
	docker build -f $< ..

arch: ArchLinux.Dockerfile
	docker build -f $< ..

fedora: Fedora.Dockerfile
	docker build -f $< ..
