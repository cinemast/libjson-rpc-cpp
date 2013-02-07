Start build process from this directory with:

$> cmake .. && make

All required libs and binaries are now in the subfuolder "build/out"
If you would like to install the libs and the stubgenerator to your system type

$> sudo make install

In UNIX based systems:
	Libraries are now installed to /usr/local/lib
	The stubgenerator ist installed to /usr/local/bin/stubgenerator

To uninstall the built libs and the stubgenerator type

$> sudo make uninstall 


