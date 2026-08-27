#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
freecadcmd="${FREECADCMD:-$HOME/Programs/bin/freecadcmd}"
generator="$repo_root/tools/freecad/generate_aircraft.py"

export ASR_AIRCRAFT_SPEC="$repo_root/config/reference-quad.aircraft.json"
export ASR_AIRCRAFT_OUTPUT_DIRECTORY="$repo_root/assets/generated"

printf "import runpy; _result = runpy.run_path(r'%s', run_name='__main__')\n" "$generator" |
  "$freecadcmd" -c
