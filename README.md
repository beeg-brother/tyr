# tyr

## Build Instructions
In order to run with `pdcurses` linking, add the `-lncurses` flag to the compilation command:
```g++ test.cpp -lncurses -lpanel```

## TODO:
- Fix text overlapping the border issue
- Write functions to focus on the editor panel
- Add file reading functionality
- Add character input functionality