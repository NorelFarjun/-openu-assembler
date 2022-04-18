#!/bin/bash

git diff-index --quiet HEAD --
PREV_CHANGES=$?

[[ ${PREV_CHANGES} != 0 ]] && git stash
git fetch upstream
git rebase upstream/master master
if [[ ${PREV_CHANGES} != 0 ]]; then
    git stash pop
    if [ "$(git ls-files -u | wc -l)" -gt 0 ] ; then
        echo "There is merge-conflict in applying your changes"
        echo "Fix those files and run 'git add .'"
        git ls-files -u
        exit 1
    fi
fi
git add .
git commit
git push
