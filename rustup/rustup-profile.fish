# Profile script to add rustup binaries to PATH upon login.

if status --is-login
    fish_add_path --global --path --append /usr/lib/rustup/bin
end
