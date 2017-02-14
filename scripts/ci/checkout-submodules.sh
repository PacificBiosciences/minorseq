#!/bin/bash
set -euo pipefail

echo "## Fetch submodules"
source /mnt/software/Modules/current/init/bash
module load git

# Bamboo's checkout of minorseq doesn't set the "origin" remote to
# something meaningful, which means we can't resolve the relative
# submodules.  Override the remote here.
git remote set-url origin ssh://git@bitbucket.nanofluidics.com:7999/sat/minorseq.git

git submodule update --init --remote
