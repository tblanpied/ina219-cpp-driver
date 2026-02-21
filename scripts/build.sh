#!/usr/bin/env bash
set -euo pipefail

# Default: host build from repo root
# Optional: build a specific example (currently: pico)
#
# Usage:
#   ./tools/build                 # host Debug
#   ./tools/build -j12            # host Debug, 12 jobs
#   ./tools/build --release       # host Release
#   ./tools/build --example pico  # pico example, Release
#   ./tools/build --example pico -j 12 --clean

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${repo_root}/build/"

jobs=""
example=""
build_type="Debug"
clean=0

die() { echo "error: $*" >&2; exit 2; }

while [[ $# -gt 0 ]]; do
  case "$1" in
    --example)
      example="${2:-}"; shift 2 ;;
    --debug)
      build_type="Debug"; shift ;;
    --release)
      build_type="Release"; shift ;;
    --clean)
      clean=1; shift ;;
    -j*)
      # supports -j12 or -j 12
      if [[ "$1" == "-j" ]]; then jobs="${2:-}"; shift 2
      else jobs="${1#-j}"; shift
      fi
      ;;
    -h|--help)
      cat <<EOF
Usage:
  ./tools/build [--debug|--release] [-jN|-j N] [--clean] [--example pico]

Notes:
  - Default is host build (repo root).
EOF
      exit 0
      ;;
    *)
      die "Unknown arg: $1" ;;
  esac
done

cmake_build_args=()
if [[ -n "${jobs}" ]]; then
  cmake_build_args+=( -j "${jobs}" )   # cmake --build supports -j/--parallel.
fi

if [[ "${example}" == "" ]]; then
  # Host build (root project)
  [[ "${clean}" == "1" ]] && rm -rf "${build_dir}"

  cmake -S "${repo_root}" -B "${build_dir}" -DCMAKE_BUILD_TYPE="${build_type}"
  cmake --build "${build_dir}" "${cmake_build_args[@]}"

elif [[ "${example}" == "pico" ]]; then
  # Pico example build (example is the root source dir for this build)
  [[ "${clean}" == "1" ]] && rm -rf "${build_dir}"

  cmake -S "${repo_root}/examples/pico" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Release
  cmake --build "${build_dir}" "${cmake_build_args[@]}"

else
  die "Unknown example: ${example} (supported: pico)"
fi
