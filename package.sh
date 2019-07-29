#!/bin/sh -e

if [ $# != 1 ]
then
   echo "Generate a binary package"
   echo "Usage: $0 <rpm>|<debian>"
   exit 1
fi




if [ "$1" = "rpm" ]
then
    ./autogen.sh && ./configure
    rpmbuild -bb rpm/mdclog.spec --define="_sourcedir $PWD" --define="_builddir $PWD" --define="_rpmdir .."
elif [ "$1" = "debian" ]
then
    ./autogen.sh && ./configure
    debuild -b -us -uc 
else
	echo Unknown argument $1
fi


