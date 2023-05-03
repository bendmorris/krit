#!/bin/bash
# zipfile_basename source_dir bin_dir

# set -x

ASSET_FILE=$3/$1.zip
[ -f "$ASSET_FILE" ]
EXISTS=$?
RSYNC_CMD="rsync -i --delete -a $2/$1 $3"

echo "running rsync command:" ${RSYNC_CMD}
RSYNC_OUTPUT=$(${RSYNC_CMD} | grep ">")

if [ -n "${RSYNC_OUTPUT}" ] || (( EXISTS != 0 )); then
    # need to rebuild or update
    FILES=$(echo "${RSYNC_OUTPUT}" | cut -d ' ' -f 2)
    if (( EXISTS == 0 )); then
        echo "updating existing assets.zip in place"
        for i in $FILES; do
            cd $3
            FILE_PATH=`realpath $i`
            echo "updating $i ($FILE_PATH)"
            echo "zip $ASSET_FILE \`realpath --relative-to=$3/$1 $FILE_PATH\`"
            cd $3/$1 && zip $ASSET_FILE `realpath --relative-to=$3/$1 $FILE_PATH`
        done
    else
        echo "rebuilding zip archive"
        cd $2 && rm -f $ASSET_FILE && cd $2/$1 && zip -r $ASSET_FILE ./*
    fi
else
    echo "no changes to assets"
fi
