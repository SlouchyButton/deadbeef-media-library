#/bin/sh

PWD=`pwd`
cd `dirname $0`
cp "./build/src/libddb_misc_media_library.so" "/home/$USER/.local/lib/deadbeef/ddb_misc_media_library.so"
cd $PWD
