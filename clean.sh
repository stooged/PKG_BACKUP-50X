#!/bin/bash
pushd tool
make clean
popd
pushd PKG_BACKUP
make clean
popd
rm -f html_payload/PKG_BACKUP.html
rm -f bin/PKG_BACKUP.bin

