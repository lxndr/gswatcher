#!/bin/sh
# Rename installed icons to have the correct name for icon theme compliance

prefix="$1"
datadir="$2"
app_id="$3"

for size in 16 24 32 48 64 128 256 512; do
  size_dir="${prefix}/${datadir}/icons/hicolor/${size}x${size}/apps"
  old_file="${size_dir}/${app_id}_${size}.png"
  new_file="${size_dir}/${app_id}.png"
  
  if [ -f "$old_file" ]; then
    mv "$old_file" "$new_file"
  fi
done
