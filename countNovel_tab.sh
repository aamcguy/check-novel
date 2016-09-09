#! /bin/bash

# to specificy you want to count the occurrence of "Novel" in field 4,
# in file PASTA.results.novel , do: 
#		./countNovel_tab.sh PASTA.results.novel 4

ifile=$1

awk -v novelcol=$2 'BEGIN { FS="\t"; cnt=0 } 
{ if( $novelcol == "Novel" ){ cnt++ }}
END { print "Novel: " cnt; print "Known: " NR - cnt }' $ifile


#NR <= 22 { next }
