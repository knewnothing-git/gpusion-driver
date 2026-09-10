# Building GPUsion from the current source tree

GPUsion is still in early development. The current repository does not contain a complete Windows driver build or installer workflow yet.

This page documents what can actually be run from the current checkout so contributors do not get blocked by commands for files that have not been added yet.

## What is available now

The repository currently contains:

- the early driver sources under `driver/`;
- a `CMakeLists.txt` file for Linux syntax checking;
- Windows driver development requirements in `CONTRIBUTING.md`.

The following files referenced elsewhere in the documentation are not present in the current tree:

- `build.ps1`
- `install.ps1`
- `scripts/build.ps1`
- `scripts/install.ps1`
- `scripts/test.ps1`

There is also no committed Visual Studio `.sln` or `.vcxproj` project for a real WDK build at this point.

## Linux syntax check

This is only a compile-time syntax check. It does not create an installable Windows driver.

Requirements:

- CMake 3.20+
- GCC
- Ninja

From the repository root:

```sh
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
```

**This check does not currently pass.** `CMakeLists.txt` enables `-Werror`, and on the current tree two files fail on format strings:

- `driver/vram/vram_proxy.c` — lines 97 and 125
- `driver/wddm/add_device.c` — lines 65, 70 and 155

The diagnostics are `format-extra-args` and `format=`. This is the already-filed [issue #7](https://github.com/knewnothing-git/gpusion-driver/issues/7), not a fault in your compiler, CMake installation or checkout. Until it is fixed, expect `libgpusion_syntax_check.a` **not** to be produced, and do not re-run the command expecting a different result: record the first error, its file and line, your compiler version, and the commit SHA.

When the tree is healthy, this build produces the static syntax-check library under `build/`. Even then it does not prove that the WDDM driver installs or that AI workloads can run through GPUsion.

## Windows development prerequisites

For real Windows driver work, `CONTRIBUTING.md` currently lists:

- Windows 10 or Windows 11, 64-bit;
- Visual Studio 2022 with the C++ desktop workload;
- Windows Driver Kit (WDK);
- Git;
- Python 3.10+ for tooling and benchmarks.

Installing those prerequisites does not make the missing build scripts appear. A real Windows build procedure can be documented once the WDK project/build files are committed.

## Before changing test-signing or security settings

Do not enable Windows test-signing mode just to inspect the repository or run the Linux syntax check. Test-signing is only relevant when there is a Windows driver binary to load for development testing.

## Current limitation

If you cloned the repository specifically to run `./build.ps1`, `./install.ps1`, or the `scripts/*.ps1` commands mentioned in the existing README/CONTRIBUTING guide, those commands cannot currently be completed because the referenced scripts are not in the repository.

The one build path that does exist — the Linux syntax check above — fails on the current tree because of the format-string errors in [issue #7](https://github.com/knewnothing-git/gpusion-driver/issues/7). So there is at present no command in this repository that completes successfully end to end. That is a property of the current commit, not of your environment, and it is worth saying plainly rather than letting contributors conclude they set something up wrongly.

Until the Windows build path lands and #7 is fixed, treat the Windows driver and the Linux syntax check both as development work in progress. Reading the sources, filing reproducible reports, and reviewing pull requests are the contributions that currently move this project forward.
