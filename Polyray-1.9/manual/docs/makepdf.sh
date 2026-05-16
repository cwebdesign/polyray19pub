#!/bin/sh
ls -al 0* |awk '{ print $9 }' >manual-files.txt
pandoc $(cat manual-files.txt) -o test.pdf --pdf-engine=typst --toc --toc-depth=4 -V mainfont="DejaVu Sans Mono"
