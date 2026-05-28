<div align="center">

```
 ██████╗ ██████╗ ██╗   ██╗███████╗██╗ ██████╗ ███╗   ██╗
██╔════╝ ██╔══██╗██║   ██║██╔════╝██║██╔═══██╗████╗  ██║
██║  ███╗██████╔╝██║   ██║███████╗██║██║   ██║██╔██╗ ██║
██║   ██║██╔═══╝ ██║   ██║╚════██║██║██║   ██║██║╚██╗██║
╚██████╔╝██║     ╚██████╔╝███████║██║╚██████╔╝██║ ╚████║
 ╚═════╝ ╚═╝      ╚═════╝ ╚══════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
```

**The GPU Illusion — AI Acceleration for Every Indian**

*Your laptop thinks it has a GPU now. It doesn't. That's the point.*

[![License: MIT](https://img.shields.io/badge/License-MIT-gold.svg)](LICENSE)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows%2010%2F11-blue.svg)]()
[![Status: Early Development](https://img.shields.io/badge/Status-Early%20Development-orange.svg)]()
[![Made in India](https://img.shields.io/badge/Made%20in-India%20🇮🇳-orange.svg)]()
[![PRD](https://img.shields.io/badge/PRD-v1.0-gold.svg)]()

</div>

---

## What is GPUsion?

GPUsion is an open-source Windows driver that **tricks your CPU and operating system into believing a GPU is present** — then routes all AI inference workloads to a highly optimized CPU engine underneath.

Every AI app you install — Ollama, Whisper, LM Studio — sees a GPU in Device Manager. They run. You pay nothing extra.

**No GPU required. No cloud subscription. No configuration hell.**

---

## The Problem We're Solving

India has **300 million+ laptops** in the ₹30,000–₹60,000 range.  
None of them have a discrete GPU.  
Every serious local AI workload requires one.

| What you want to run | What it needs | What it costs |
|---|---|---|
| Llama 3 8B locally | 8GB VRAM | ₹25,000+ GPU |
| Stable Diffusion | RTX 3060 min | ₹22,000+ GPU |
| Whisper (speech → text) | GPU preferred | 5× slower on CPU |
| Local coding assistant | GPU for real-time | ₹800–2,000/mo cloud |

The barrier isn't intelligence. It's access.  
GPUsion removes the barrier.

---

## How It Works

```
┌─────────────────────────────────────────────────────┐
│  YOUR AI APP  (Ollama / Whisper / LM Studio / etc.) │
│         calls standard GPU APIs as normal           │
└──────────────────────┬──────────────────────────────┘
                       │  DirectML / Vulkan / OpenCL calls
                       ▼
┌─────────────────────────────────────────────────────┐
│            GPUSION VIRTUAL DRIVER                   │
│   Windows believes this is a real GPU adapter       │
│   "GPUsion Virtual Adapter — 8GB VRAM" in Device   │
│   Manager. DXGI enumeration. WDDM 2.x compliant.   │
└──────────────────────┬──────────────────────────────┘
                       │  routed inference workloads
                       ▼
┌─────────────────────────────────────────────────────┐
│           INFERENCE ENGINE                          │
│   llama.cpp · ONNX Runtime · AVX2/AVX-512 SIMD     │
│   INT4/INT8 quantization · CPU RAM as VRAM proxy   │
└─────────────────────────────────────────────────────┘
```

The illusion is at the driver level. The performance is real.

---

## Quick Start

> ⚠️ **GPUsion is in early development.** This is not yet ready for production use.  
> Star the repo and watch for our first release.

```bash
# Coming in Phase 1 — Month 4
# One-click installer. No command line required.
# gpusion-setup.exe
```

For developers who want to build from source:

```bash
git clone https://github.com/gpusion/gpusion-driver
cd gpusion-driver

# Enable test signing mode (development only)
# Run as Administrator:
bcdedit /set testsigning on

# Build (requires Windows Driver Kit)
./build.ps1

# Install driver
./install.ps1
```

---

## Repositories

| Repo | Description | Status |
|---|---|---|
| [`gpusion-driver`](.) | WDDM virtual GPU kernel driver — the core illusion | 🔨 Active |
| [`gpusion-inference`](.) | CPU inference translation layer (llama.cpp / ONNX) | 🔨 Active |
| [`gpusion-installer`](.) | One-click Windows installer for non-technical users | 📋 Planned |
| [`gpusion-firmware`](.) | FPGA/ASIC firmware for Phase 2 hardware dongle | 📋 Phase 2 |
| [`gpusion-benchmark`](.) | Standardized benchmarks vs. real GPU baselines | 📋 Planned |

---

## Supported AI Frameworks (Phase 1 Target)

| Framework | Priority | Status |
|---|---|---|
| [Ollama](https://ollama.ai) | Critical | 🔨 In progress |
| [llama.cpp](https://github.com/ggerganov/llama.cpp) | Critical | 🔨 In progress |
| [ONNX Runtime DirectML](https://onnxruntime.ai) | High | 📋 Planned |
| [Whisper.cpp](https://github.com/ggerganov/whisper.cpp) | High | 📋 Planned |
| [AUTOMATIC1111](https://github.com/AUTOMATIC1111/stable-diffusion-webui) | Phase 2 | 🔮 Future |

---

## Roadmap

```
Month 1–2   ██████░░░░░░░░░░  Driver skeleton — WDDM adapter in Device Manager
Month 2–3   ████████░░░░░░░░  Ollama integration — first LLM runs end-to-end  
Month 3–4   ██████████░░░░░░  One-click installer — zero config for non-technical users
Month 4–5   ████████████░░░░  PUBLIC LAUNCH — GitHub + HackerNews + Reddit
Month 5–6   ██████████████░░  Benchmarks published — honest numbers vs. real GPU
Month 6–9   ████████████████  PHASE 2 — FPGA hardware dongle POC
Month 9–12  ████████████████  Hardware v1 — GPUsion Stick @ ₹2,499
Month 12–24 ████████████████  PHASE 3 — Custom ASIC, Made in India
```

---

## Performance Expectations (Honest)

GPUsion Phase 1 (software) is **not a GPU replacement**. It is a GPU *emulator* that makes local AI possible on hardware that previously couldn't run it at all.

| Model | Real RTX 3060 | GPUsion Phase 1 | GPUsion Phase 2 (FPGA) |
|---|---|---|---|
| Llama 3 8B (INT4) | ~50 tok/s | ~6–10 tok/s | ~20–30 tok/s (est.) |
| Whisper Base | Real-time | ~0.5× real-time | ~0.9× real-time (est.) |
| Stable Diffusion | ~15 s/img | Not Phase 1 | ~60 s/img (est.) |

*Slow is better than impossible.*  
*Local is better than cloud.*  
*Free is better than ₹2,000/month.*

---

## Why Open Source?

GPUsion's driver and inference layer are **MIT licensed** — completely free, forever.

The software is the proof. The hardware dongle (Phase 2) is the product.

We believe:
- Indian developers deserve infrastructure they can trust, audit, and improve
- Open source creates the community moat that makes this impossible to kill
- A working virtual GPU driver with real traction is the best pitch to any hardware partner

---

## Contributing

GPUsion is built by one person and one AI.  
It needs you to become something bigger.

**Where help is most needed right now:**

- 🔧 **Windows Driver Kit experience** — WDDM 2.x kernel driver development
- 🧠 **llama.cpp / ONNX Runtime** — inference optimization on CPU
- 🧪 **Testing** — Intel / AMD laptop compatibility across generations
- 📝 **Documentation** — Hindi + English setup guides for non-technical users
- 🌐 **Translations** — Hindi, Tamil, Telugu, Bengali READMEs

**Bounties available:**
- ₹5,000 — First working DirectML → CPU passthrough
- ₹5,000 — Successful Ollama model run via GPUsion driver
- ₹2,500 — Compatibility report for any new CPU/laptop model
- ₹500 — Documentation improvements

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details.

---

## Technical Deep Dive

For the full architecture, phase specifications, risk analysis, and commercial strategy see the **[Product Requirements Document](docs/PRD_v1.0.md)**.

Key technical decisions:

**Why WDDM and not a user-space shim?**  
Kernel-mode registration is required for apps that query the DirectX device list at startup. User-space intercepts miss these queries and apps refuse to run. WDDM is the only path to true OS-level illusion.

**Why not just optimize CPU inference directly?**  
We do that too — but without the virtual GPU layer, apps that check for GPU presence at startup simply refuse to launch. The illusion enables the inference, not the other way around.

**Why INT4 quantization by default?**  
A 7B parameter model in FP16 needs ~14GB RAM. In INT4 it needs ~4GB. Most budget Indian laptops have 8GB RAM. INT4 is not a compromise — it is the product.

---

## The Name

**GPU + Illusion = GPUsion**

Inspired by **Mohini** — the only female avatar of Vishnu, who created a perfect illusion to trick the Asuras from drinking Amrit.

GPUsion creates a perfect GPU illusion.  
The Asuras are the hardware bottleneck.  
The Amrit is local AI.

---

## License

MIT License — see [`LICENSE`](LICENSE)

The driver is free. The story is Indian. The mission is access.

---

<div align="center">

**Built in India 🇮🇳 for India — and everyone else who deserves local AI**

*If this resonates, star the repo. That's how this story grows.*

⭐ **Star** · 🍴 **Fork** · 🐛 **Issue** · 💬 **Discuss**

</div>
