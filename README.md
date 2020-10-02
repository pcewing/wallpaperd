# wallpaperd - Wallpaper Daemon

An X11 application that periodically chooses a new random wallpaper for every
monitor by detecting screen dimensions. The primary motivation behind the
application was to familiarize with X11 and specifically `xcb`.

As a user of `i3wm` and `compton` this application is being written with those
in mind. It probably won't work under other window managers, compositors, or
desktop environments.

**WIP Warning:** This project is still heavily work in progress.

## Configuration

Configuration is done via a yaml file and is searched for based on the
XDG spec:
```
---
search_paths:
    - "/home/john/Pictures/wallpapers"
rotation:
    enabled: true
    frequency: 120
```

## Dependencies:

- xcb
- sqlite3
- yaml
- flatbuffers\*
- stb\_image\*\*

\* The source for this is cloned and built as part of the build process (See
[Makefile](./Makefile))
\*\*This is a single header library and is checked directly into the source tree

