#!/bin/bash
iconv -f UTF-32LE -t UTF-8 utf32le.txt > utf8.txt
iconv -f UTF-32LE -t UTF-16LE utf32le.txt > utf16le.txt
iconv -f UTF-32LE -t UTF-16BE utf32le.txt > utf16be.txt
iconv -f UTF-32LE -t UTF-32BE utf32le.txt > utf32be.txt
