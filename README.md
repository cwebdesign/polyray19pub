"# polyray19pub" 

Polyray — Open Source Revival (MIT License)

Polyray is a classic 1990s raytracer originally created by Alexander Enzmann.
It was distributed as shareware and became known for its flexibility, scene
description language, and surprisingly advanced rendering features for its era.

This repository is an officially permitted open-source revival of Polyray,
released under the MIT License at the request of the original author.

## Goals of the Revival

- Preserve and modernize a historically significant raytracer
- Make the code accessible for learning, experimentation, and research
- Provide a clean, buildable version for modern systems
- Encourage community contributions while respecting the original design

##  Repository Structure
Polyray-1.9/        # Updated source code (with MIT headers)  
- docs/             # Manuals, historical notes, technical references
- examples/         # Sample scenes and materials
- tools/            # Utilities, converters, helper scripts (TODO)
CREDITS.md        # Attribution and acknowledgements
LICENSE           # MIT License
README.md         # This file
CHANGELOG.md      # Version history for the revival



This is an updated version of the classic Polyray raytracer updated to compile under Windows

## Building
Git clone this repository, and cd into the directory

Install 64-bit MSYS2 to compile under Windows: https://github.com/msys2/msys2/wiki/MSYS2-installation

Then update your pacman as follows: pacman -Syuu

Following that, install the prerequisites:

pacman -S base-devel

pacman -S mingw-w64-x86_64-toolchain

(choose 1,2,3,4,5,8,9,10)

pacman -S mingw-w64-x86_64-boost

pacman -S mingw-w64-x86_64-sdl3

## Void Linux

You will need the SDL library for newer beta versions. 

xbps-install SDL3

xbps-install SDL3-devel

You also need RE-flex. This needs to be compiled from source as per the instructions and then installed to /usr/local.

You also need the boost development library. This is how to install it (this information isn't that easy to find online)

xbps-install boost

xbps-install boost-build 

xbps-install boost-devel

## macos

Using g++ from homebrew. Clang is not tested, sorry!

brew install gcc

brew install re-flex

brew install sdl3

## Documentation

The `docs/` folder contains:

- Original Polyray documentation
- Notes on the revival process (TODO)
- Technical details about the scene language and renderer

A beta GPT tutor is available:
https://chatgpt.com/g/g-JVuMbYmDl-polyray-mentor

## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.

## Contributing

Contributions are welcome!  
Please keep changes respectful to the original architecture unless the goal is
explicit modernisation.

## License

Polyray is released under the MIT License with the explicit permission of the
original author. See `LICENSE` for details.

## SAST Tools

[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
