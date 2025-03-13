# *et*, egg timer

## What?

A minimal *(egg)* timer based on
[libnotify](https://developer.gnome.org/libnotify/).


## Screenshot?

![et screenshot](example.jpg)

You're seeing the [Dunst-notification daemon](http://www.knopwob.org/dunst/)
running on [dwm](http://dwm.suckless.org/).


## How?

To start an **et**, just fire a command.

```bash
$ et 90 # Reminds you in 90 seconds
$ et 1:30 # Also reminds you in 90 seconds
$ et 1:30:00 # Reminds you in 90 minutes
$ et 10:00 Pizza # Reminds you in 10 minutes and says Pizza
$ et 23:00 "Check rice" # Reminder with a longer text
```

To get the current status, send a `SIGUSR1`-signal to the process.
Don't worry, it won't stop.

```bash
$ et 120
Egg timer started!
Name: eggtimer, PID: 4242
Only 30 seconds remaining

$ kill -SIGUSR1 4242
01 minutes and 28 seconds remaining
```

You can also get the status of all **et**-instances by running
the `et-status.sh`-script.

Perhaps you want to set up some keybindings in your WM for a
flawless experience.


## Build?

### Debian / Ubuntu

```bash
sudo apt-get update
sudo apt-get install build-essential libnotify-dev

make
```

### NixOS / Nix

If you're using [Nix](https://nixos.org/nix/) or [NixOS](https://nixos.org/)
try `nix-build` or just install the `et` package from the
[nixpkgs](https://github.com/NixOS/nixpkgs).

### OpenBSD

```
doas pkg_add libnotify

make
```
