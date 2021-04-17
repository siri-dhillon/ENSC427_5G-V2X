#!/bin/sh
apt-get update  # To get the latest package lists
apt-get install g++ python3 python3-dev pkg-config sqlite3
apt-get install python3-setuptools git
apt-get install qt5-default mercurial
apt-get install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3
apt-get install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
apt-get install autoconf cvs bzr unrar
apt-get install gdb valgrind 
apt-get install uncrustify
apt-get install doxygen graphviz imagemagick
apt-get install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk
apt-get install python3-sphinx dia 
apt-get install gsl-bin libgsl-dev libgslcblas0 libgslcblas0 libgslcblas0:i386
apt-get install tcpdump
apt-get install libsqlite3-dev
apt-get install libxml2 libxml2-dev
apt-get install libgtk-3-dev
apt-get install vtun lxc uml-utilities
apt-get install cmake libc6-dev libc6-dev-i386 automake python3-pip
python3 -m pip install --user cxxfilt
apt-get install libclang-dev llvm 
