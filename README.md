# HKGDL Geode Mod

A Geode mod for Geometry Dash that integrates the [Hong Kong Geometry Dash Demon List (HKGDL)](https://hkgdl.dpdns.org) directly into the game.

## Features

- **Automatic Detection**: When viewing a level info page, the mod automatically checks if the level is on the HKGD Demon List
- **Victors Button**: If the level is on the list, a "HKGD Victors" button appears under the difficulty indicator
- **Victors Popup**: Click the button to see all victors with their rank, username, and completion date
- **Position Display**: Shows the level's HKGD position (e.g., "HKGD #5")

## Installation

1. Install [Geode](https://geode-sdk.org/) for Geometry Dash
2. Download the latest release from the [Releases](https://github.com/HKGDL/hkgdl-geode-mod/releases) page
3. Place the `.geode` file in your Geometry Dash `mods` folder
4. Restart the game

## Building from Source

### Prerequisites

- [Geode SDK](https://geode-sdk.org/) installed
- CMake 3.21+
- C++23 compatible compiler

### Build Steps

```bash
cd geode/HKGDL-Intergration
geode build
```

The compiled mod will be in `build-win/hkgd-community.hkgdl-intergration.geode`.

## Configuration

The mod has a configurable API URL setting:

1. Open Geometry Dash
2. Go to Geode settings
3. Find "HKGDL-Intergration" in the mod list
4. Configure the API URL if needed (default: `https://api.hkgdl.dpdns.org/api`)

## How It Works

1. When you open a level's info page, the mod queries the HKGDL API to check if the level is on the list
2. If found, it creates a button showing the level's position
3. Clicking the button fetches and displays all victors in a scrollable popup

## API Endpoints Used

- `GET /levels` - Fetches all levels with their records to find matches by `levelId`

## Screenshot

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/e8726f11-e3ee-4b99-906d-cee04d248d75" />
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/b87dfb13-f94f-43b8-a009-dbdba9a3a194" />



## Credits

- HKGDL Community
- [Geode SDK](https://geode-sdk.org/)

## License

[Dont Be A Dick (DBAD) Public License](DBAD.md)

## Links

- [HKGD Demon List](https://hkgdl.dpdns.org)
- [HKGD API](https://api.hkgdl.dpdns.org/api)
- [Geode SDK](https://geode-sdk.org/)
