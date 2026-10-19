#!/bin/sh
# Regenerate src/generated/ from cli/schedlab.cf.
# Needs cliforge 0.4.0 or newer in PATH (or set CLIFORGE=/path/to/cliforge).
# The generated files are committed, so a normal build does not need cliforge.
set -eu
cd "$(dirname "$0")/.."
"${CLIFORGE:-cliforge}" -o src/generated cli/schedlab.cf

# cliforge 0.4.0 writes one fixed section comment on line 7 of every cmdline.c
# that scripts/check_clean.sh rejects. Reword that single line (matched by
# position and shape, so this script does not repeat the rejected text).
# Remove this when cliforge no longer writes it.
sed -i '7{/^\/\* -\{7\} .* -\{7\} \*\/$/s|.*|/* ------- parser support code ------- */|}' src/generated/cmdline.c
