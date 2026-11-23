
# Gluten Free Burger and Fries

Not one, not two, but a whole three-dimensional game console for the PIC24.

See issues.

# Quick-start digital

## Emulator

**Linux:**
```bash
cd digital
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cd build
make -j $(nproc) && ./gfbaf_sdl
```

**Windows (untested):**
```bash
cd digital
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\path\to\sdl3
cd build
make -j $(nproc) && ./gfbaf_sdl
````
