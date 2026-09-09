# Building GPUsion from source

GPUsion is still in early development, and the repository does **not yet contain a production Windows build/install script or a complete WDK project**. This page documents what can be built from the current tree today so contributors do not lose time following commands for files which are not present yet.

## What works today

The checked-in `CMakeLists.txt` is a **Linux syntax-check build**, not a functional Windows GPU driver build. It compiles the current driver C sources against the compatibility stubs and is the same basic path used by CI.

On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y cmake gcc g++ ninja-build cppcheck

git clone https://github.com/knewnothing-git/gpusion-driver.git
cd gpusion-driver

cmake -B build -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

This should produce the `gpusion_syntax_check` static-library target. It verifies that the current C sources compile cleanly with the Linux compatibility layer; it does **not** produce a loadable Windows driver.

To run the same static analysis used by CI:

```bash
cppcheck \
  --enable=all \
  --suppress=missingIncludeSystem \
  --suppress=unusedFunction \
  --error-exitcode=1 \
  --inline-suppr \
  -I driver \
  -I driver/compat \
  -D__linux__ \
  driver/
```

## Windows / WDK status

A real GPUsion driver build will require Windows, Visual Studio/MSBuild, and the Windows Driver Kit. The CI file contains the intended future Windows build shape, but that job is currently commented out and the repository does not yet include the referenced `scripts/build.ps1`, `scripts/install.ps1`, or a complete `driver/gpusion.vcxproj` workflow.

Until those files land, commands which invoke those scripts should be treated as roadmap documentation rather than working setup instructions.

If you are contributing the Windows build plumbing itself, a useful first milestone is to make this command real and reproducible on a clean Windows runner:

```powershell
msbuild driver/gpusion.vcxproj /p:Configuration=Debug /p:Platform=x64
```

A corresponding PR should include the project file, any required WDK configuration, and exact prerequisites so another contributor can reproduce the build from a fresh checkout.

## Repository areas that exist today

The current driver source tree includes:

```text
driver/
├── compat/      # Linux compatibility stubs used by syntax-check CI
├── dxgi/        # DXGI-related source
├── kmdf/        # KMDF driver source
├── vram/        # virtual-VRAM source
├── wddm/        # WDDM driver source
├── gpusion.h
└── gpusion.inf
```

The top-level CI workflow is `.github/workflows/ci.yml`, and the Linux syntax-check build is defined in `CMakeLists.txt`.

## Before opening a build-related PR

Please verify that:

1. every command in the documentation references a file which actually exists in the PR;
2. Linux syntax-check CI still passes;
3. Windows-specific commands clearly distinguish between tested commands and planned commands;
4. a new contributor can tell whether an output is merely a syntax-check artifact or a loadable Windows driver.

Keeping that distinction explicit is especially important for a driver project: a successful CMake syntax check is useful evidence, but it is not evidence that Windows can install, enumerate, or execute the driver.
