#!/bin/sh
# Install the git pre-commit hook that runs scripts/check_clean.sh.
# Run once after cloning.
set -eu
cd "$(git rev-parse --show-toplevel)"

hook="$(git rev-parse --git-path hooks)/pre-commit"
mkdir -p "$(dirname "$hook")"
cat >"$hook" <<'EOF'
#!/bin/sh
exec "$(git rev-parse --show-toplevel)/scripts/check_clean.sh"
EOF
chmod +x "$hook"
echo "installed $hook"
