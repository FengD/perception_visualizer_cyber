## Description

* The project include all the common codes and some usefull scritps.

## Build

* Set env for different platform
`export PLATFORM=X86`  `export TAG=1604`  `export TAG=1804` for X86

`./build.sh` build the project.
`./build.sh clean` clean the build.
run `./build.sh help` to see the details.


## Release
* If the build successed, all the release are in the `build_dist` folder.

### help menu
./build.sh help

```
Usage:
    ./build.sh [OPTION]

Options:
    all: run all
    build: run the code build
    clean: clean the code build
    cov: run the code test coverage
    check_code: run the code style check
```

