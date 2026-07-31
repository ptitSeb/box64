# The guide on running Steam with Box64

Steam works with Box64, both Linux version and Windows version. Systems with less than 6GB RAM may need a swap file for optimal performance.

## Install and run Linux Steam

Linux Steam requires Box86 or Box32 and it's binfmt enabled, check [Compilation Instructions](COMPILE.md) for details.

```bash
# This file can be found in the project root directory
./install_steam.sh

# And then, just run
steam
```

For additional stability with Box64 when Box86 is unavailable, it is recommended to use the new 64-bit-only runtime for the Steam client.

- Steam > Settings > Interface > Client Beta Participation: Steam Beta > Restart Now
- Steam > Settings > Interface > Use experimental SteamRT3 Steam Client: On > Restart Now

## Install and run Windows Steam

It's simple, just download it from Steam website and install it like other Windows applications.
