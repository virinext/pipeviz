#!/bin/sh

set -e

version_file=$1
out_file=$2

major=`sed -n '1p' $version_file`
minor=`sed -n '2p' $version_file`

commit=`git rev-list --count HEAD 2>/dev/null || echo 0`
if [ x"$commit" = 0 ]; then
    date=0
else
    date=`date +'%d%m'%y`
fi

mkdir -p -- "${out_file%/*}"
t=$(mktemp "$out_file".XXXXXXXX)
trap 'rm -f -- "$out_file"' EXIT

echo "#ifndef VERSION_INFO_H_"                                      > $t
echo "#define VERSION_INFO_H_"                                      >> $t
echo ""                                                             >> $t
echo "#define VERSION_STR \"$major.$minor.$commit.$date\""          >> $t
echo ""                                                             >> $t
echo "#endif"                                                       >> $t

if test -e "$out_file" && cmp -s "$out_file" "$s"; then
    rm -- "$t"
else
    mv -f -- "$t" "$out_file"
fi
trap - EXIT
