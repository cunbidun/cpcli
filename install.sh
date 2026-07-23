#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_result="${CPCLI_INSTALL_RESULT:-$repo_root/result}"
prefix="${CPCLI_INSTALL_PREFIX:-$HOME/.local}"

if [ ! -d "$build_result" ]; then
	echo "Missing build result at $build_result" >&2
	echo "Run: nix build" >&2
	exit 1
fi

if [ ! -d "$build_result/bin" ] || [ ! -d "$build_result/share/cpcli" ]; then
	echo "Build result does not look like cpcli install output: $build_result" >&2
	exit 1
fi

mkdir -p "$prefix/bin" "$prefix/share"

chmod u+w "$prefix/bin" "$prefix/share" 2>/dev/null || true
if [ -d "$prefix/share/cpcli" ]; then
	chmod -R u+w "$prefix/share/cpcli" 2>/dev/null || true
fi
chmod -R u+w "$prefix/bin" 2>/dev/null || true
for bin in cpcli_app cpcli_cc cpcli_editor; do
	if [ -e "$prefix/bin/$bin" ] || [ -L "$prefix/bin/$bin" ]; then
		chmod u+w "$prefix/bin/$bin" 2>/dev/null || true
	fi
done

rm -rf "$prefix/share/cpcli"
rm -f "$prefix/bin/cpcli_app" "$prefix/bin/cpcli_cc" "$prefix/bin/cpcli_editor"

cp -a "$build_result/bin/." "$prefix/bin/"
cp -a "$build_result/share/cpcli" "$prefix/share/"
chmod -R u+w "$prefix/bin"
chmod -R u+w "$prefix/share/cpcli"
chmod u+w "$prefix/bin/cpcli_app" "$prefix/bin/cpcli_cc" 2>/dev/null || true

echo "Installed cpcli to $prefix"
