Krit is a 2D game engine written in C++.

Features:

- Batching SDL/OpenGL renderer
- Flexible rendering pipeline makes custom shaders, render to texture, etc. painless
- Entity component system
- XML layouts with responsive layout engine
- Behavior trees
- Utilities for loading and parsing static game data files
- Scoped asset manager for textures, sound, data files and custom asset types
- Text rendering with UTF-8 and rich text support, via freetype and harfbuzz
- Built-in support for Spine skeletal animation
- Play sounds or stream music via OpenAL
- Cross-platform support: Linux, Windows (cross-compile via mingw-w64), and Emscripten (WIP)
- Load assets from filesystem, with ZIP archive support
- Scripting
    - Script in JavaScript or TypeScript using the ultra-lightweight QuickJS
    - Painless native interoperability: write Typescript type definitions to generate binding code for native types automatically
    - JSX support for generating layouts
