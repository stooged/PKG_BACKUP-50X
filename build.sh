#!/bin/bash
set -e
pushd tool
make
popd
pushd PKG_BACKUP
make
popd
mkdir -p bin
rm -f bin/PKG_BACKUP.bin
cp PKG_BACKUP/PKG_BACKUP.bin bin/PKG_BACKUP.bin
mkdir -p html_payload
tool/bin2js bin/PKG_BACKUP.bin > html_payload/payload.js
FILESIZE=$(stat -c%s "bin/PKG_BACKUP.bin")
PNAME=$"PKG_BACKUP"
cp exploit.template html_payload/PKG_BACKUP.html
sed -i -f - html_payload/PKG_BACKUP.html << EOF
s/#NAME#/$PNAME/g
s/#BUF#/$FILESIZE/g
s/#PAY#/$(cat html_payload/payload.js)/g
EOF
rm -f PKG_BACKUP/PKG_BACKUP.bin
rm -f html_payload/payload.js
