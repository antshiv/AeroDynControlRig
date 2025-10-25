#!/bin/bash

# AeroDynControlRig Documentation Viewer
# This script generates and opens the Doxygen documentation in your default browser

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== AeroDynControlRig Documentation Generator ===${NC}"
echo ""

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo -e "${YELLOW}Warning: Doxygen is not installed${NC}"
    echo "Install it with:"
    echo "  Ubuntu/Debian: sudo apt install doxygen graphviz"
    echo "  macOS: brew install doxygen graphviz"
    exit 1
fi

# Check if Graphviz (dot) is installed
if ! command -v dot &> /dev/null; then
    echo -e "${YELLOW}Warning: Graphviz (dot) is not installed${NC}"
    echo "Documentation will be generated without diagrams."
    echo "Install Graphviz with:"
    echo "  Ubuntu/Debian: sudo apt install graphviz"
    echo "  macOS: brew install graphviz"
fi

# Generate documentation
echo -e "${GREEN}Generating documentation...${NC}"
doxygen Doxyfile

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Documentation generated successfully!${NC}"
    echo ""

    # Find the index.html file
    INDEX_FILE="docs/doxygen/html/index.html"

    if [ -f "$INDEX_FILE" ]; then
        echo -e "${BLUE}Documentation available at:${NC}"
        echo "  $(pwd)/$INDEX_FILE"
        echo ""

        # Open in browser
        echo -e "${GREEN}Opening documentation in browser...${NC}"

        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            xdg-open "$INDEX_FILE" 2>/dev/null || echo "Please open $INDEX_FILE manually"
        elif [[ "$OSTYPE" == "darwin"* ]]; then
            open "$INDEX_FILE"
        elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
            start "$INDEX_FILE"
        else
            echo -e "${YELLOW}Unsupported OS. Please open $INDEX_FILE manually${NC}"
        fi

        echo ""
        echo -e "${BLUE}Quick Navigation:${NC}"
        echo "  Main Page         → Overview and quick start"
        echo "  Architecture      → Detailed diagrams (OpenGL, ImGui, data flow)"
        echo "  Classes           → All 23+ documented classes"
        echo "  Class Hierarchy   → Visual inheritance tree"
        echo "  Files             → Browse source code"
        echo ""
        echo -e "${GREEN}Key Features to Explore:${NC}"
        echo "  • Click on class names to see inheritance/collaboration diagrams"
        echo "  • View call graphs for functions (what calls what)"
        echo "  • Browse Architecture Guide for OpenGL/ImGui explanations"
        echo "  • Use search box (top-right) to find anything"
        echo ""
    else
        echo -e "${YELLOW}Error: Documentation index not found at $INDEX_FILE${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}Error: Documentation generation failed${NC}"
    exit 1
fi
