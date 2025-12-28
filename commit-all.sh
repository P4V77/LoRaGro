#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(pwd)"

# Repositories relative to root
REPOS=(
  "FW-LoRaGro/app/Fino-LoRaGro"
  "FW-LoRaGro/app/Gano-LoRaGro"
  "FW-LoRaGro/modules/loragro_module"
  "Common-LoRaGro"
  "Hardware-LoRaGro"
  "SW-LoRaGro/Gapp-LoRaGro"
)

# Optional commit message
COMMIT_MSG="${1:-Initial commit}"

for r in "${REPOS[@]}"; do
  echo "────────────────────────────────────────"
  echo "Processing: $r"

  if [[ ! -d "$r" ]]; then
    echo "⚠️  Directory not found, skipping"
    continue
  fi

  cd "$r"

  if [[ ! -d ".git" ]]; then
    echo "⚠️  Not a git repository, skipping"
    cd "$ROOT_DIR"
    continue
  fi

  git add .

  if git diff --cached --quiet; then
    echo "ℹ️  No changes to commit"
  else
    git commit -m "$COMMIT_MSG ($r)"
    echo "✅ Committed"
  fi

  cd "$ROOT_DIR"
done

echo "────────────────────────────────────────"
echo "All repositories processed."
