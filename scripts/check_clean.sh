#!/bin/sh
# Must pass before every commit (scripts/install-hooks.sh wires it as a hook).
# Fails if:
#   1. anything under local/ is tracked or staged, or
#   2. a word from the git-ignored list local/denylist.txt appears in a
#      tracked or staged file.
# It prints file names only, never the matched words.
set -eu
cd "$(git rev-parse --show-toplevel)"

fail=0

# 1. local/ must never reach git. (.gitignore keeps it out; this is the safety net.)
if [ -n "$(git ls-files -- local)" ]; then
    echo "check_clean: files under local/ are tracked or staged:" >&2
    git ls-files -- local >&2
    fail=1
fi

# 2. Deny-list words. Compare against the index (what would be committed).
deny=local/denylist.txt
if [ -f "$deny" ]; then
    patterns=$(mktemp)
    trap 'rm -f "$patterns"' EXIT
    # One word per line; skip blank lines and # comments (a blank pattern would match everything).
    grep -v -e '^[[:space:]]*$' -e '^[[:space:]]*#' "$deny" >"$patterns" || true
    if [ -s "$patterns" ]; then
        hits=$(git grep --cached -l -I -i -w -F -f "$patterns" || true)
        if [ -n "$hits" ]; then
            echo "check_clean: deny-list word found in:" >&2
            echo "$hits" >&2
            fail=1
        fi
    fi
else
    echo "check_clean: note: $deny not found, deny-list check skipped" >&2
fi

if [ "$fail" -ne 0 ]; then
    echo "check_clean: FAILED" >&2
    exit 1
fi
echo "check_clean: ok"
