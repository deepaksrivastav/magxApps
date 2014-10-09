#!/bin/sh
MYFILE=`basename $0`
MYPATH=`echo $0 | sed -e 's/'$MYFILE'//g'`
HOME=$MYPATH
MEDIAFILE="$ZDIRNAME/$IFILENAME"

filename="${MEDIAFILE##*/}"                      # Strip longest match of */ from start
dir="${MEDIAFILE:0:${#MEDIAFILE} - ${#filename}}" # Substring from 0 thru pos of filename
base="${filename%.[^.]*}"                       # Strip shortest match of . plus at least one non-dot char from end
ext="${filename:${#base} + 1}"                  # Substring from len of base thru end
if [[ -z "$base" && -n "$ext" ]]; then          # If we have an extension and no base, it's really the base
    base=".$ext"
    ext=""
fi

if [ -d "/mmc/mmca1/.cache/$MEDIAFILE" ]; then
	ACTUALFILE = "${filename}""_ind.html"
	/usr/SYSqtapp/ljbrowser/browser -d /mmc/mmca1/.cache/$filename/$ACTUALFILE.html
else 
	mkdir /mmc/mmca1/.cache/$filename
	$MYPATH/pdftohtml -c $MEDIAFILE /mmc/mmca1/.cache/$filename/$filename
	ACTUALFILE = "${filename}""_ind.html"
	/usr/SYSqtapp/ljbrowser/browser -d /mmc/mmca1/.cache/$filename/$ACTUALFILE.html
fi