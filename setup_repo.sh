#!/bin/bash
# setup_repo.sh — GPUsion First Commit Push Script
#
# Run this once on your desktop/laptop to push the complete
# driver skeleton to github.com/knewnothing-git/gpusion-driver
#
# Prerequisites:
#   - git installed
#   - GitHub account: knewnothing-git
#   - Either: GitHub CLI (gh) installed and authenticated
#   - Or: Personal Access Token (PAT) with repo scope
#
# Usage:
#   chmod +x setup_repo.sh
#   ./setup_repo.sh
#
# Built in India 🇮🇳

set -e  # Exit immediately on any error

REPO_URL="https://github.com/knewnothing-git/gpusion-driver.git"
COMMIT_MSG="feat: initial driver skeleton — WDDM virtual GPU adapter

GPUsion Phase 1 driver skeleton. Registers as a WDDM 2.7 compliant
virtual display adapter in Windows Device Manager. Intercepts GPU API
calls and routes to CPU inference engine.

Implemented:
- DriverEntry + DDI table registration (driver_entry.c)
- Device context lifecycle — AddDevice/RemoveDevice (add_device.c)
- Device start/stop — StartDevice/StopDevice (start_device.c)
- Adapter capability reporting — QueryAdapterInfo (query_adapter_info.c)
- VRAM proxy — CreateAllocation/DestroyAllocation (vram_proxy.c)
- DXGI child enumeration — compute-only adapter (dxgi_enum.c)
- Command submission + inference routing (submit_command.c)
- Backend detection — CPU/FPGA/ASIC auto-select (backend_detect.c)
- Windows API stubs for Linux CI syntax checking (win_stubs.h)
- Driver installation manifest (gpusion.inf)
- GitHub Actions CI pipeline (syntax check + static analysis)
- MIT License

Phase 2 (FPGA dongle) and Phase 3 (custom ASIC) stubs included.
All TODOs marked clearly for contributor pickup.

Built in India 🇮🇳 — https://github.com/knewnothing-git/gpusion-driver"

echo ""
echo "  ██████╗ ██████╗ ██╗   ██╗███████╗██╗ ██████╗ ███╗  ██╗"
echo "  ██╔════╝ ██╔══██╗██║   ██║██╔════╝██║██╔═══██╗████╗ ██║"
echo "  ██║  ███╗██████╔╝██║   ██║███████╗██║██║   ██║██╔██╗██║"
echo "  ██║   ██║██╔═══╝ ██║   ██║╚════██║██║██║   ██║██║╚████║"
echo "  ╚██████╔╝██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚███║"
echo "   ╚═════╝ ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝╚═╝  ╚══╝"
echo ""
echo "  First Commit Push — github.com/knewnothing-git/gpusion-driver"
echo ""

# ─── Check prerequisites ─────────────────────────────────────────────────

echo "→ Checking prerequisites..."

if ! command -v git &> /dev/null; then
    echo "  ✗ git not found. Install from https://git-scm.com"
    exit 1
fi
echo "  ✓ git found: $(git --version)"

# ─── Initialise repo ─────────────────────────────────────────────────────

echo "→ Initialising git repository..."

# If we're already in a git repo, just use it
if [ ! -d ".git" ]; then
    git init
    echo "  ✓ git init done"
else
    echo "  ✓ existing git repo found"
fi

# ─── Configure git identity if not set ───────────────────────────────────

if [ -z "$(git config user.email)" ]; then
    echo "→ Git identity not configured. Setting defaults..."
    git config user.email "gpusion@users.noreply.github.com"
    git config user.name "GPUsion"
    echo "  ✓ Git identity set (update with: git config user.name 'Your Name')"
fi

# ─── Stage all files ─────────────────────────────────────────────────────

echo "→ Staging files..."
git add -A

FILE_COUNT=$(git diff --cached --name-only | wc -l)
echo "  ✓ ${FILE_COUNT} files staged"

# Show what's being committed
echo ""
echo "  Files in first commit:"
git diff --cached --name-only | sed 's/^/    /'
echo ""

# ─── Commit ──────────────────────────────────────────────────────────────

echo "→ Creating commit..."
git commit -m "$COMMIT_MSG"
echo "  ✓ Commit created"

# ─── Set remote ──────────────────────────────────────────────────────────

echo "→ Setting remote origin..."
if git remote get-url origin &>/dev/null; then
    git remote set-url origin "$REPO_URL"
    echo "  ✓ Remote origin updated"
else
    git remote add origin "$REPO_URL"
    echo "  ✓ Remote origin added"
fi

# ─── Push ────────────────────────────────────────────────────────────────

echo "→ Pushing to GitHub..."
echo "  (You may be prompted for GitHub credentials)"
echo "  Tip: Use a Personal Access Token as your password"
echo "  Create one at: https://github.com/settings/tokens"
echo ""

git branch -M main
git push -u origin main

# ─── Done ────────────────────────────────────────────────────────────────

echo ""
echo "  ✅ GPUsion is live!"
echo ""
echo "  🔗 https://github.com/knewnothing-git/gpusion-driver"
echo ""
echo "  Next steps:"
echo "  1. Add a GitHub repository description:"
echo "     'Open-source virtual GPU driver for AI acceleration — Made in India 🇮🇳'"
echo "  2. Add topics: windows-driver, gpu, ai, wddm, directml, india, open-source"
echo "  3. Pin the repository on your GitHub profile"
echo "  4. Post on r/LocalLLaMA and HackerNews"
echo ""
echo "  Built in India 🇮🇳"
echo ""
