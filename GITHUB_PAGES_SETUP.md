# GitHub Pages Setup Guide

This guide explains how to enable GitHub Pages for your Doxygen documentation.

## One-Time Setup (5 minutes)

### Step 1: Update README Badges

Replace `yourusername` in `README.md` with your actual GitHub username:

```markdown
[![Doxygen](https://github.com/YOURUSERNAME/AeroDynControlRig/actions/workflows/doxygen.yml/badge.svg)](https://github.com/YOURUSERNAME/AeroDynControlRig/actions/workflows/doxygen.yml)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://YOURUSERNAME.github.io/AeroDynControlRig/)
```

Also update the documentation link:
```markdown
📚 **[View Documentation](https://YOURUSERNAME.github.io/AeroDynControlRig/)** - Complete system architecture with visual diagrams
```

### Step 2: Push to GitHub

```bash
git add .github/workflows/doxygen.yml
git add README.md
git commit -m "Add GitHub Actions workflow for Doxygen documentation"
git push origin main
```

### Step 3: Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **Settings** (top right)
3. Click **Pages** (left sidebar)
4. Under "Build and deployment":
   - Source: **GitHub Actions** (NOT "Deploy from a branch")
5. Click **Save**

### Step 4: Wait for First Deploy

1. Go to **Actions** tab in your repo
2. You should see "Deploy Doxygen Documentation" workflow running
3. Wait ~2-3 minutes for it to complete (green checkmark)
4. Visit: `https://yourusername.github.io/AeroDynControlRig/`

## What Happens Automatically

After setup, every time you push to `main`:

1. ✅ GitHub Actions runs Doxygen
2. ✅ Generates HTML documentation
3. ✅ Deploys to GitHub Pages
4. ✅ Documentation is live within 2-3 minutes

## How to Access Documentation

**Live URL**: `https://yourusername.github.io/AeroDynControlRig/`

**What's Included**:
- Complete system architecture
- Visual SVG diagrams (rendering pipeline, OpenGL buffers, quaternion conversion)
- Class hierarchy and collaboration diagrams
- Source code browser with syntax highlighting
- Search functionality
- Module documentation

## Manual Trigger

You can manually trigger documentation rebuild:

1. Go to **Actions** tab
2. Click "Deploy Doxygen Documentation"
3. Click **Run workflow** dropdown
4. Click **Run workflow** button

## Troubleshooting

### "Pages deployment failed"

Check that you selected **GitHub Actions** (not "Deploy from a branch") in Settings → Pages.

### "Workflow doesn't run"

1. Check `.github/workflows/doxygen.yml` exists
2. Make sure you pushed to `main` branch
3. Check Actions tab for error messages

### "Documentation URL shows 404"

1. Wait 2-3 minutes after first deploy
2. Check Actions tab to ensure deployment completed successfully
3. Verify Settings → Pages shows the correct URL

### "Images/SVGs don't load"

1. Verify `docs/images/` contains the SVG files
2. Check Doxyfile has `IMAGE_PATH = docs/images`
3. Regenerate docs locally to test: `doxygen Doxyfile`

## Benefits

✅ **Always up-to-date** - Auto-regenerates on every push
✅ **Professional** - Shows you write well-documented code
✅ **Free hosting** - GitHub Pages is free for public repos
✅ **Easy sharing** - Just send the URL to collaborators
✅ **Portfolio piece** - Employers can see your documentation skills

## Example Repositories Using This

- [ImGui](https://github.com/ocornut/imgui) - Uses similar setup
- [SFML](https://github.com/SFML/SFML) - Game development library
- [Eigen](https://gitlab.com/libeigen/eigen) - Linear algebra library

Your documentation is now on par with professional open-source projects! 🎉
