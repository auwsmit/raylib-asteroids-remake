# Asteroids Remake with raylib

<p align="center">
  <a href="https://auwsmit.github.io/raylib-asteroids-remake/">
    <img style="width:800px" src="https://github.com/auwsmit/raylib-asteroids-remake/blob/main/demo.gif">
  </a>
  <p align="center">
    <a href="https://auwsmit.github.io/raylib-asteroids-remake/"><b>Click here to play in web browser!</b></a>
  </p>
</p>

This is my second [raylib](https://www.raylib.com/) project, made to learn
about the library and basic game development. Like before, there were many
features and some bugs that I didn't get to before my self-imposed deadline.
But I'm pretty happy with how it turned out.

Runs on Windows, Linux, MacOS, and web browsers.

## Controls

- **Select/Confirm:** `Mouse click`/`Enter`/`Space`

- **Back/Cancel:** `Esc`/`Backspace`

- **Ship Controls:**
    - **Shoot:** `Left click`/`Space`
    - **Thrust:** `Right click`/`W`/`↑`
    - **Aim:** `Move mouse`/`A`/`D`/`←`/`→`

- **Pause:** `P`/`Esc`

- **Toggle fullscreen:** `Alt+Enter`/`F11`/`Shift+F` (desktop only)

## Build for Desktop
1. Build by running `./build.sh cmake` or `.\build.bat cmake`, depending on your platform
    - Alternatively, just run `make`, or the build script with no arguments.
2. Play by running `./asteroids` or `.\asteroids.exe`

## Build for Browser
1. Same as desktop, but add `web` as an argument:
    - Run `build.sh cmake web` or `make web`
2. Play by running `emrun asteroids.html`

## Requirements to build:

- Library: [raylib](https://www.raylib.com/), duh :P
- A C compiler: [GCC](https://gcc.gnu.org/), [Clang](https://clang.llvm.org/) ([llvm-mingw](https://github.com/mstorsjo/llvm-mingw)
on Windows), or [Visual Studio](https://visualstudio.microsoft.com/)
    - [emscripten](https://emscripten.org/) (only for browser / web assembly compilation)
- Build system: [CMake](https://cmake.org/) or [Make](https://en.wikipedia.org/wiki/Make_(software)) (macOS requires CMake)

For Windows users with no build tools or less experience, [w64devkit](https://github.com/skeeto/w64devkit) is a fast and easy way to build this project. Just download and extract it anywhere. Then use the included unix-like terminal, or add the `w64devkit/bin` tools to your PATH. Now you can simply `make` this project like you would on Linux. As an added bonus, you now have access to many nice Unix command line tools.

