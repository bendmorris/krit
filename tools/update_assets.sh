#!/bin/bash
echo "running rsync command:" rsync -i --delete -a $1/assets $2
RSYNC_OUTPUT=$(rsync -i --delete -a $1/assets $2)
if [ -n "${RSYNC_OUTPUT}" ]; then
    # updated assets
    echo "updated assets; rebuilding zip archive"
    cd $2 && rm -f assets.zip && zip -r assets.zip assets
else
    echo "no changes to assets"
fi
