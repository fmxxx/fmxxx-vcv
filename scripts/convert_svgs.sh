#!/usr/bin/env bash
set -euo pipefail

SRC_DIR="res-src"
DEST_DIR="res"
mkdir -p "$DEST_DIR"

INKSCAPE_NS="http://www.inkscape.org/namespaces/inkscape"

for src_file in "$SRC_DIR"/*.svg; do
    [[ -e "$src_file" ]] || continue
    filename=$(basename "$src_file")
    dest_file="$DEST_DIR/$filename"
    echo "Processing: $src_file -> $dest_file"

    # 1) Text -> paths. No --export-plain-svg, so the file is written back
    #    as-is with only the <text> elements swapped for <path>. All
    #    inkscape:* attributes (including the label) are kept.
    inkscape --export-text-to-path --export-filename="$dest_file" "$src_file"

    # 2) Hide the "components" layer by rewriting its style attribute.
    style=$(xmlstarlet sel -N inkscape="$INKSCAPE_NS" \
              -t -v "//*[@inkscape:label='components']/@style" "$dest_file")

    if [[ -z "$style" ]]; then
        echo "  Error: no 'components' layer in $dest_file" >&2
        continue
    fi

    if [[ "$style" == *"display:"* ]]; then
        new_style=$(sed -E 's/display:[^;]*/display:none/' <<< "$style")
    else
        new_style="display:none;$style"
    fi

    xmlstarlet ed --inplace -N inkscape="$INKSCAPE_NS" \
        -u "//*[@inkscape:label='components']/@style" -v "$new_style" \
        "$dest_file"
done
