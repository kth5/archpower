#!/bin/sh
LANG_PREFIX=$(expr substr "$LANG" 1 2)
if [ ! -d "/usr/share/netsurf/$LANG_PREFIX" ]; then
  # Use English if a directory for the current language does not exist
  export LANG=en_US.UTF-8
fi
/usr/bin/netsurf-gtk3 "$@"
