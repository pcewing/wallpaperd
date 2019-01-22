# wallpaperd - Wallpaper Daemon

TODO: Add description

## TODO List

- Triage issue where some wallpapers don't update correctly
- Address all missing documentation in header files
- Support a config file (YAML?)
    - Consider looking at how netplan deserializes yaml
- Make coding style consistent
- Address missing error handling
- Improve logging
- Replace all instances of `WPD_ERROR_TODO` with real error codes

## Dependencies:
```
- xcb
- sqlite3
- stb_image*

*This is a single header library and is checked directly into the source tree
```

