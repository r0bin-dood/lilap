lilap (pronounced like you would lilac), it's a lightweight access point (AP) management tool for GNU/Linux.
It's severly limited in capabilities, especially compared to the hostap family of tools. Its primary objective is the management and presentation of a captive portal.

lilap is a minimal-dependencies project, which means there's a lot of wheel re-inventing. It's a one stop solution and does not involve other projects like NGINX, create_ap, hostapd, ifconfig, etc.

## Build
Run `cargo build` and `cargo run` to build and clean up lilap respectively

## Usage
lilap only takes 1 argument. Any other arguments will be ignored.

`$ lilap <config_file>`

See [docs/config_file.md](docs/config_file.md)