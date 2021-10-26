#!/bin/bash
ASSET_FILE=$2/assets.zip
[ -f "$ASSET_FILE" ]
EXISTS=$?
RSYNC_CMD="rsync -i --delete -a $1/assets $2"

echo "running rsync command:" ${RSYNC_CMD}
RSYNC_OUTPUT=$(${RSYNC_CMD} | grep ">")

if [ -n "${RSYNC_OUTPUT}" ] || (( EXISTS != 0 )); then
    # need to rebuild or update
    FILES=$(echo "${RSYNC_OUTPUT}" | cut -d ' ' -f 2)
    if (( EXISTS == 0 )); then
        echo "updating existing assets.zip in place"
        for i in $FILES; do
            echo "updating $i"
            cd $2 && zip -r $ASSET_FILE $i
        done
    else
        echo "rebuilding zip archive"
        cd $2 && rm -f $ASSET_FILE && zip -r $ASSET_FILE assets
    fi
else
    echo "no changes to assets"
fi
