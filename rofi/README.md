# rofi

## Silent ABI bump on new relases and plugins rebuilds

New rofi releases may include *silent* ABI bumps which require all the rofi plugin packages to be rebuilt.  
As a matter of precaution, it is encouraged to always rebuild all rofi plugins / reverse dependencies against new rofi releases.

Creating ToDos to track those rebuilds (in `staging`) is encouraged. For instance: <https://archlinux.org/todo/rofi-plugins-rebuild-against-rofi-176/>.
