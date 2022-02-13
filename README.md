Why not lua?
 - JS is C-like and looks cleaner
 - Troubles with vapi. I needed to write more fucntion so might as well write duktape vpa file from scratch
 - Some operation are trivial in JS like object merging and shortcuts

GSETTINGS_SCHEMA_DIR=tmp/schemas

glib-compile-schemas --targetdir=tmp/schemas src/schemas
