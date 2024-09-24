#!/bin/bash
echo "Commit message: "
read commit_message
if [ -z "$commit_message" ]; then
	echo "Error: No commit message"
	exit 1
fi
git add .
git commit -m $commit_message
git push
exit 0
