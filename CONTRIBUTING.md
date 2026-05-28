# Contributing to GPUsion

First — thank you. GPUsion is built by one person and one AI. Every contribution moves this closer to the hands of 300 million Indians who deserve local AI.

This document tells you everything you need to know to contribute effectively.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Where Help Is Needed Most](#where-help-is-needed-most)
- [Bounty Program](#bounty-program)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Submit a Contribution](#how-to-submit-a-contribution)
- [Coding Standards](#coding-standards)
- [Issue Labels](#issue-labels)
- [Testing Requirements](#testing-requirements)
- [Community](#community)

---

## Code of Conduct

GPUsion is an Indian-built open source project. We hold two things above all else:

1. **Respect** — for contributors of every skill level, background, and location
2. **Honesty** — about what works, what doesn't, and what we don't know yet

Harassment, gatekeeping, or elitism of any kind will result in immediate removal. A student in Nagpur contributing their first kernel patch deserves the same respect as a 20-year Windows driver veteran.

We follow the [Contributor Covenant v2.1](https://www.contributor-covenant.org/version/2/1/code_of_conduct/).

---

## Where Help Is Needed Most

### 🔴 Critical (Phase 1 blockers)

| Area | What's needed | Skill required |
|---|---|---|
| WDDM Kernel Driver | Complete KMD skeleton — device registration, DXGI enumeration, fake VRAM reporting | C, Windows Driver Kit (WDK) |
| DirectML Intercept | Hook DirectML API calls at the driver level, route to ONNX Runtime CPU | C++, DirectML SDK |
| Ollama Compatibility | Ensure GPUsion virtual adapter is selected by Ollama auto-detection | C, GPU enumeration |

### 🟡 High Priority

| Area | What's needed | Skill required |
|---|---|---|
| llama.cpp Integration | Wire llama.cpp as the inference backend for LLM workloads | C++, llama.cpp internals |
| AVX2 Optimisation | Profile and optimise matrix operations on Intel/AMD CPUs | C, SIMD intrinsics |
| INT4 Quantisation | Automatic model quantisation pipeline on install | Python, GGUF format |
| Whisper.cpp Support | Speech-to-text inference routing via GPUsion driver | C++, whisper.cpp |

### 🟢 Good First Issues

| Area | What's needed | Skill required |
|---|---|---|
| Documentation | Hindi + English setup guides for non-technical users | Writing, Hindi |
| Compatibility Testing | Test GPUsion on your laptop, file a detailed report | No code — just a laptop |
| Translations | README in Hindi, Tamil, Telugu, Bengali, Marathi | Language skills |
| Benchmark Scripts | Automated performance measurement scripts | Python |
| Installer UX | NSIS/WiX installer script with zero-config flow | NSIS or WiX |

---

## Bounty Program

GPUsion pays contributors for milestone achievements. Bounties are paid via UPI or bank transfer (India) or Wise (international).

| Bounty | Amount | Criteria |
|---|---|---|
| First working DirectML → CPU passthrough | ₹5,000 | PR merged, verified working on 2+ laptops |
| Successful Ollama model run via GPUsion | ₹5,000 | PR merged, Llama model completes inference |
| Whisper.cpp working via GPUsion driver | ₹3,000 | PR merged, real-time factor > 0.3× |
| Compatibility report — new laptop model | ₹500 | Filed using the compatibility report template |
| Documentation — Hindi setup guide | ₹1,000 | Reviewed and merged |
| Documentation improvement (any) | ₹500 | Reviewed and merged |
| Bug report with reproduction steps | ₹250 | Confirmed reproducible by maintainer |

To claim a bounty: open an issue tagged `bounty-claim` with your PR link and payment details. Bounties are paid within 7 days of PR merge.

> Bounty pool is limited. First merged PR for each milestone wins.

---

## Getting Started

### Prerequisites

You will need:

- **Windows 10 or 11** (64-bit) — GPUsion is Windows-first
- **Visual Studio 2022** with C++ desktop workload
- **Windows Driver Kit (WDK)** — [Download from Microsoft](https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk)
- **Git** for Windows
- **Python 3.10+** for tooling and benchmark scripts

For inference work only (no driver development):
- Any OS with Python 3.10+, llama.cpp or ONNX Runtime

### Enable Test Signing Mode

All driver development requires test signing mode. Run PowerShell as Administrator:

```powershell
bcdedit /set testsigning on
# Restart Windows
```

You will see a small watermark on your desktop. This is expected. To revert:

```powershell
bcdedit /set testsigning off
# Restart Windows
```

> ⚠️ Never distribute drivers built in test signing mode. They will not load on other machines.

---

## Development Setup

```bash
# 1. Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/gpusion-driver
cd gpusion-driver

# 2. Add the upstream remote
git remote add upstream https://github.com/gpusion/gpusion-driver

# 3. Create a branch for your work
git checkout -b feature/your-feature-name

# 4. Build the driver (requires WDK installed)
./scripts/build.ps1

# 5. Install the driver (run as Administrator)
./scripts/install.ps1

# 6. Verify installation
# Open Device Manager → Display Adapters
# You should see "GPUsion Virtual Adapter"
```

### Repository Structure

```
gpusion-driver/
├── driver/
│   ├── kmdf/           # Kernel Mode Driver Framework code
│   ├── wddm/           # WDDM display miniport driver
│   ├── dxgi/           # DXGI adapter enumeration
│   └── vram/           # Virtual VRAM management
├── inference/
│   ├── directml/       # DirectML API intercept layer
│   ├── onnx/           # ONNX Runtime CPU backend
│   └── llama/          # llama.cpp integration
├── installer/
│   ├── nsis/           # NSIS installer scripts
│   └── assets/         # Installer UI assets
├── tests/
│   ├── unit/           # Unit tests
│   ├── integration/    # End-to-end tests
│   └── compat/         # Compatibility test scripts
├── benchmarks/         # Performance measurement scripts
├── docs/               # Technical documentation
└── scripts/            # Build and install scripts
```

---

## How to Submit a Contribution

### For Bug Reports

1. Search existing issues first — it may already be reported
2. Open a new issue using the **Bug Report** template
3. Include:
   - Windows version (`winver` output)
   - CPU model and generation
   - RAM amount
   - Exact error message or behaviour
   - Steps to reproduce
   - What you expected vs. what happened

### For Feature Requests

1. Open an issue using the **Feature Request** template
2. Describe the problem you're solving, not just the solution
3. Wait for maintainer feedback before building — saves wasted effort

### For Code Contributions

```
1. Open or find an issue for your change
2. Comment "I'm working on this" to avoid duplicate effort
3. Fork → branch → code → test → PR
4. Fill out the PR template completely
5. Link the PR to its issue: "Closes #123"
6. Wait for review — we aim to respond within 48 hours
```

### PR Requirements

Every PR must:

- [ ] Pass all existing tests (`./scripts/test.ps1`)
- [ ] Include tests for new functionality
- [ ] Update documentation if behaviour changes
- [ ] Follow the coding standards below
- [ ] Have a clear, descriptive title
- [ ] Include a description of what changed and why

PRs that touch the kernel driver require testing on at minimum:
- One Intel CPU laptop (any generation)
- One AMD CPU laptop (if available)

---

## Coding Standards

### C / C++ (Driver and Inference Code)

```c
// Function names: snake_case
// Types: PascalCase  
// Constants: UPPER_SNAKE_CASE
// Private members: _leading_underscore

// Every function must have a comment block
/**
 * gpusion_register_adapter
 * Registers the virtual display adapter with the WDDM subsystem.
 * 
 * @param device_context  Pointer to the GPUsion device context
 * @return NTSTATUS       STATUS_SUCCESS or error code
 */
NTSTATUS gpusion_register_adapter(GPUSION_DEVICE_CONTEXT* device_context);
```

- No magic numbers — every constant gets a named `#define`
- Every `malloc` / allocation has a corresponding free path
- Error paths must be explicit — no silent failures
- Windows API errors must be checked — no `(void)` casting return values

### Python (Tooling and Benchmarks)

- Python 3.10+ type hints on all functions
- `black` formatter — run before every commit
- `ruff` linter — zero warnings required
- Docstrings on every function

### Commit Messages

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat(driver): register virtual DXGI adapter with 8GB fake VRAM
fix(inference): correct AVX2 path selection on older Intel CPUs
docs(readme): add Hindi translation of quick start section
test(compat): add compatibility report for Lenovo IdeaPad Gen 7
chore(ci): add Windows Server 2022 to test matrix
```

---

## Issue Labels

| Label | Meaning |
|---|---|
| `good-first-issue` | No prior kernel experience needed |
| `bounty` | Paid bounty available — see amount in issue |
| `driver-core` | Touches kernel mode driver code |
| `inference` | CPU inference engine work |
| `compatibility` | Laptop/CPU compatibility work |
| `docs` | Documentation only |
| `bug` | Confirmed bug with reproduction steps |
| `needs-testing` | PR needs testing on more hardware |
| `phase-2` | Hardware dongle work — not yet active |

---

## Testing Requirements

### Unit Tests

Located in `tests/unit/`. Run with:

```powershell
./scripts/test.ps1 -suite unit
```

### Integration Tests

Require a working driver installation. Run with:

```powershell
./scripts/test.ps1 -suite integration
```

Integration tests verify:
- Driver loads and registers in Device Manager
- DXGI enumeration returns GPUsion adapter
- DirectML inference completes without error
- Memory is correctly released after inference

### Compatibility Reports

If you don't write code but want to contribute — **compatibility reports are extremely valuable**.

Run the compatibility script on your laptop:

```powershell
./scripts/compat-report.ps1
```

This generates a report file. Open an issue using the **Compatibility Report** template and attach it.

We especially need reports from:
- Intel 10th, 11th, 12th, 13th, 14th gen CPUs
- AMD Ryzen 5000, 6000, 7000 series
- Laptops with 4GB, 6GB, 8GB, 16GB RAM
- Windows 10 21H2+, Windows 11 all versions

---

## Community

- **GitHub Discussions** — architecture questions, ideas, general chat
- **Issues** — bug reports and feature requests only

Language: English or Hindi both welcome in all community spaces.

We are a small team. Response time is 24–48 hours on weekdays, slower on weekends. We appreciate patience.

---

## Recognition

Every merged contributor is listed in [`CONTRIBUTORS.md`](CONTRIBUTORS.md).

Significant contributors (3+ merged PRs) are listed in the README.

Bounty recipients are acknowledged in release notes.

---

*GPUsion is built for the student in Nagpur, the freelancer in Coimbatore, the small business owner in Surat. Every line of code you contribute moves AI access closer to someone who deserves it.*

*Built in India 🇮🇳 — with gratitude to everyone who builds with us.*
