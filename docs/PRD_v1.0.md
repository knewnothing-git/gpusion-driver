# GPUsion — Product Requirements Document
## Technical Specification v1.0

**Status:** Active Development  
**Version:** 1.0  
**Date:** May 2026  
**Authors:** GPUsion Core Team  

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Problem Statement](#2-problem-statement)
3. [Product Vision](#3-product-vision)
4. [User Personas](#4-user-personas)
5. [System Architecture](#5-system-architecture)
6. [Phase 1 — Software Driver Specification](#6-phase-1--software-driver-specification)
7. [Phase 2 — Hardware Dongle Specification](#7-phase-2--hardware-dongle-specification)
8. [Phase 3 — Custom ASIC Specification](#8-phase-3--custom-asic-specification)
9. [Open Source Strategy](#9-open-source-strategy)
10. [Commercial Model](#10-commercial-model)
11. [Driver Signing Strategy](#11-driver-signing-strategy)
12. [Performance Targets](#12-performance-targets)
13. [Roadmap](#13-roadmap)
14. [Risk Register](#14-risk-register)
15. [Success Metrics](#15-success-metrics)
16. [Glossary](#16-glossary)

---

## 1. Executive Summary

GPUsion is an open-source AI acceleration platform for Windows. It consists of a kernel-mode virtual GPU driver that registers as a legitimate display adapter with the Windows operating system, combined with an optimized CPU inference engine underneath.

The result: every AI application that checks for GPU presence — Ollama, LM Studio, Whisper, ONNX Runtime apps — finds one. GPUsion's inference engine handles the actual compute. The application never knows the difference.

**The core mechanic in one sentence:**  
*GPUsion creates a GPU illusion at the OS level so that AI workloads run on hardware that was previously unable to run them.*

### Why This Exists

India has over 300 million laptops in the ₹30,000–₹60,000 price range. None of them ship with a discrete GPU. Every meaningful local AI workload — local LLMs, speech recognition, image generation — requires one. Cloud APIs cost ₹800–2,000/month. The configuration complexity of existing solutions (CUDA, WSL2, driver stacks) puts local AI out of reach for the majority of users even if they could afford the hardware.

GPUsion solves the access problem, not just the cost problem.

### Strategic Approach

```
Phase 1  →  Open-source software driver  →  Prove the mechanic, build community
Phase 2  →  FPGA hardware dongle         →  Real dedicated inference silicon
Phase 3  →  Custom ASIC chip             →  Made in India, sub-₹2,500 MSRP
```

The software layer is MIT-licensed and free forever. The hardware is the commercial product.

---

## 2. Problem Statement

### 2.1 The Hardware Access Gap

| Use Case | Minimum GPU Required | Approximate Cost | GPUsion Phase |
|---|---|---|---|
| Llama 3 8B local inference | 8GB VRAM | ₹25,000+ | Phase 1 |
| Whisper speech-to-text | GPU strongly preferred | 5× slower without | Phase 1 |
| Local coding assistant | GPU for real-time speed | ₹800–2,000/mo cloud | Phase 1 |
| Stable Diffusion | RTX 3060 minimum | ₹22,000+ | Phase 2 |

### 2.2 The Configuration Gap

Even users who acquire a GPU face:

- CUDA version mismatches with installed frameworks
- WSL2 configuration on Windows requiring command-line expertise
- VRAM allocation errors that are opaque to non-technical users
- Framework-specific driver conflicts (PyTorch vs. TensorFlow vs. ONNX)

A non-technical user's success rate with current local AI setup guides is below 30%. GPUsion's one-click installer targets 95%+ first-attempt success.

### 2.3 The Sovereignty Gap

Every rupee spent on cloud AI APIs (OpenAI, Google, Anthropic) leaves India. GPUsion is infrastructure for AI compute sovereignty at the individual level — the same compute runs locally, privately, and free of ongoing subscription cost.

---

## 3. Product Vision

### 3.1 Vision Statement

> "Every Indian with a laptop should be able to run AI locally — for free, without a GPU, without cloud dependency, without configuration hell."

### 3.2 Design Principles

**Principle 1 — The illusion must be complete**  
If a single app can detect that GPUsion is not a real GPU, the product has failed. The virtual adapter must pass every standard GPU enumeration check that AI frameworks perform.

**Principle 2 — Zero configuration for end users**  
One installer. One click. No terminal. No driver conflicts. No CUDA version selection. If a non-technical user in Tier 3 India cannot install it in under 5 minutes, the UX has failed.

**Principle 3 — Honest about limitations**  
GPUsion Phase 1 is slower than a real GPU. This is stated clearly in all documentation and benchmarks. Trust is built through honesty, not marketing.

**Principle 4 — Open source is the moat, not the sacrifice**  
The driver and inference layer are MIT-licensed not because we are altruistic but because open source creates the community, the credibility, and the acquisition interest that a closed product cannot.

**Principle 5 — Hardware is the premium layer**  
The software proves the mechanic. The hardware dongle (Phase 2) is the product people pay for. The ASIC (Phase 3) is the IP moat.

---

## 4. User Personas

### Persona 1 — Arjun (Primary)
- **Age:** 20 | **Location:** Tier 2 city | **Device:** ₹45,000 laptop, Intel i5, 8GB RAM, no GPU
- **Goal:** Run a local coding assistant for college projects without paying for cloud APIs
- **Blocker:** Tried setting up Ollama three times. Failed at CUDA configuration each time.
- **GPUsion value:** Ollama auto-detects GPUsion virtual adapter. Model runs. No configuration.

### Persona 2 — Priya
- **Age:** 28 | **Location:** Mumbai | **Device:** ₹55,000 laptop, AMD Ryzen 5, 16GB RAM
- **Goal:** Use Stable Diffusion for client design work without Midjourney subscription
- **Blocker:** Cannot afford ₹22,000 GPU. Cloud image gen latency breaks creative workflow.
- **GPUsion value:** Phase 2 hardware dongle enables image generation at acceptable speed.

### Persona 3 — Rajan
- **Age:** 35 | **Location:** Surat | **Device:** Office laptop, Intel i3, 4GB RAM
- **Goal:** Transcribe customer calls in Hindi using local Whisper
- **Blocker:** Zero technical knowledge. Cannot navigate any existing setup guide.
- **GPUsion value:** One-click installer. Whisper works. He never knew a driver was involved.

### Persona 4 — Deepa
- **Age:** 24 | **Location:** Hyderabad | **Device:** ₹40,000 laptop, Intel i5, 8GB RAM
- **Goal:** Use local AI for research without sending sensitive documents to cloud APIs
- **Blocker:** Privacy concern. Distrust of cloud data handling for academic research.
- **GPUsion value:** Everything runs locally. No data leaves the machine.

---

## 5. System Architecture

### 5.1 Layered Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER                        │
│                                                                 │
│   Ollama · Whisper · LM Studio · AUTOMATIC1111 · Any           │
│   DirectML / Vulkan / OpenCL application                       │
│                                                                 │
│   ← These applications are NEVER modified. Zero changes.       │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            │  Standard GPU API calls
                            │  (DirectML / Vulkan / OpenCL / DXGI)
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                    GPUSION VIRTUAL DRIVER                       │
│                                                                 │
│   ┌─────────────────┐  ┌──────────────────┐  ┌─────────────┐  │
│   │  WDDM KMD       │  │  API Interceptor  │  │  Device     │  │
│   │  Kernel Mode    │  │  DirectML hooks   │  │  Emulator   │  │
│   │  Driver         │  │  Vulkan layer     │  │  DXGI enum  │  │
│   └─────────────────┘  └──────────────────┘  └─────────────┘  │
│                                                                 │
│   ┌─────────────────┐  ┌──────────────────┐                    │
│   │  VRAM Proxy     │  │  Command         │                    │
│   │  RAM as VRAM    │  │  Translator      │                    │
│   │  management     │  │  GPU ops → CPU   │                    │
│   └─────────────────┘  └──────────────────┘                    │
│                                                                 │
│   ← Windows Device Manager shows:                              │
│     "GPUsion Virtual Adapter — 8192MB VRAM"                   │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            │  Routed inference workloads
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│                      INFERENCE ENGINE                           │
│                                                                 │
│   Phase 1 (Software)                                           │
│   ┌──────────────┐  ┌──────────────┐  ┌─────────────────────┐ │
│   │  llama.cpp   │  │ ONNX Runtime │  │  OpenBLAS           │ │
│   │  LLM backend │  │ CPU backend  │  │  BLAS operations    │ │
│   └──────────────┘  └──────────────┘  └─────────────────────┘ │
│   AVX2 / AVX-512 SIMD  ·  INT4 / INT8 quantisation            │
│                                                                 │
│   Phase 2 (Hardware)                                           │
│   ┌──────────────────────────────────────────────────────────┐ │
│   │  FPGA Systolic Array  ·  USB 3.2 Gen 2  ·  10Gbps       │ │
│   └──────────────────────────────────────────────────────────┘ │
│                                                                 │
│   Phase 3 (ASIC)                                               │
│   ┌──────────────────────────────────────────────────────────┐ │
│   │  Custom RISC-V + 256-PE Systolic Array  ·  TSMC 28nm    │ │
│   └──────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### 5.2 Data Flow

```
1. App queries DirectX device list on startup
   → GPUsion WDDM adapter appears in enumeration
   → App selects "GPUsion Virtual Adapter" as compute device

2. App submits inference workload via DirectML / ONNX
   → GPUsion API interceptor receives the workload
   → Command translator converts GPU tensor ops to CPU ops

3. Inference engine executes workload
   → Phase 1: llama.cpp / ONNX Runtime on CPU with AVX2
   → Phase 2: Offloads to FPGA via USB, returns result
   → Phase 3: Offloads to ASIC, returns result

4. Result returned to app via standard API response
   → App receives output as if a real GPU processed it
   → App never detects the substitution
```

---

## 6. Phase 1 — Software Driver Specification

### 6.1 WDDM Kernel Mode Driver

**Purpose:** Register GPUsion as a legitimate Windows Display Driver Model adapter.

**Requirements:**

| Requirement | Specification |
|---|---|
| WDDM version | 2.7 minimum (Windows 10 2004+) |
| Driver type | Kernel Mode Driver (KMD) |
| Framework | Windows Driver Framework (WDF) |
| Language | C (C99) |
| Build system | Windows Driver Kit (WDK) + MSBuild |
| Signing (development) | Test signing mode |
| Signing (production) | EV certificate or Microsoft open source signing |

**Virtual adapter properties reported to Windows:**

```c
// Reported to DXGI / DirectX enumeration
#define GPUSION_ADAPTER_NAME        L"GPUsion Virtual Adapter"
#define GPUSION_VENDOR_ID           0x6750  // 'GP' — not a real vendor ID
#define GPUSION_DEVICE_ID           0x0001
#define GPUSION_VRAM_BYTES          (8ULL * 1024 * 1024 * 1024)  // 8GB
#define GPUSION_SHARED_MEMORY_BYTES (8ULL * 1024 * 1024 * 1024)  // 8GB shared
#define GPUSION_DRIVER_VERSION      0x00010000  // 1.0.0.0
```

**Key DDI (Device Driver Interface) implementations required:**

```
DxgkDdiStartDevice          — Initialize virtual hardware
DxgkDdiStopDevice           — Clean shutdown
DxgkDdiQueryAdapterInfo     — Report adapter capabilities to OS
DxgkDdiCreateAllocation     — VRAM allocation (proxied to system RAM)
DxgkDdiBuildPagingBuffer    — Memory transfer operations
DxgkDdiSubmitCommand        — Receive and queue GPU commands
DxgkDdiPatch                — Patch command buffers
DxgkDdiPresent              — Display present (stub — no display output)
```

### 6.2 DirectML Intercept Layer

**Purpose:** Intercept DirectML inference calls before they reach the (non-existent) real GPU and reroute them to the CPU inference engine.

**Interception strategy:**

DirectML uses COM interfaces. GPUsion registers a COM proxy that sits between the application's DirectML calls and the actual DirectML runtime. When an app creates a `IDMLDevice`, it receives GPUsion's proxy device. All subsequent inference operations are captured, translated, and executed on CPU.

```
App calls:     DMLCreateDevice(d3dDevice, flags, &dmlDevice)
GPUsion returns: gpusion_dml_device (COM proxy)
App calls:     dmlDevice->CreateOperatorInitializer(...)
GPUsion translates to: onnxruntime::Session with CPU execution provider
```

### 6.3 Inference Engine

**Primary backend: llama.cpp** (for LLM workloads)

| Parameter | Value |
|---|---|
| Default quantisation | INT4 (Q4_K_M) |
| Fallback quantisation | INT8 (Q8_0) |
| Thread count | Physical CPU cores − 1 |
| Context length | 4096 tokens default (configurable) |
| Memory strategy | mmap for model weights, minimise RAM pressure |

**Secondary backend: ONNX Runtime CPU** (for non-LLM workloads)

| Parameter | Value |
|---|---|
| Execution provider | CPUExecutionProvider |
| SIMD optimisation | AVX2 (required) / AVX-512 (optional) |
| Thread pool | OMP thread pool, bound to physical cores |
| Memory arena | Pre-allocated, configurable size |

**Quantisation pipeline:**

When a user installs a model that is not already quantised, GPUsion automatically converts it to INT4 on first run:

```
1. Detect model format (GGUF, SafeTensors, ONNX)
2. Check available RAM
3. Select quantisation level:
   - 4GB  RAM → Q4_K_S (smallest)
   - 8GB  RAM → Q4_K_M (balanced) ← default
   - 16GB RAM → Q5_K_M or Q8_0
4. Convert and cache quantised model
5. Load quantised model for inference
```

### 6.4 VRAM Proxy

GPU applications allocate VRAM for model weights and activation tensors. GPUsion proxies these allocations into system RAM with intelligent management:

```
Allocation request → check available system RAM
  If available     → allocate in RAM, return virtual VRAM address
  If insufficient  → trigger automatic model quantisation to fit
  If still tight   → page least-recently-used tensors to disk
```

**VRAM proxy limits by system RAM:**

| System RAM | Reported VRAM | Usable VRAM proxy | Recommended max model |
|---|---|---|---|
| 4GB | 8GB | ~2GB | 3B parameters INT4 |
| 8GB | 8GB | ~4GB | 7B parameters INT4 |
| 16GB | 8GB | ~8GB | 13B parameters INT4 |
| 32GB | 8GB | ~16GB | 34B parameters INT4 |

### 6.5 Supported Frameworks — Phase 1

| Framework | Integration method | Target milestone |
|---|---|---|
| Ollama | WDDM adapter enumeration + llama.cpp backend | Month 2–3 |
| llama.cpp (direct) | Direct backend integration | Month 2–3 |
| ONNX Runtime DirectML | COM proxy intercept | Month 3–4 |
| Whisper.cpp | Direct backend integration | Month 3–4 |
| LM Studio | WDDM adapter enumeration | Month 4–5 |
| AUTOMATIC1111 | Phase 2 target — requires image gen performance | Phase 2 |

### 6.6 Installer Specification

**Requirement:** A non-technical user with no prior knowledge must be able to install GPUsion in under 5 minutes, with no command-line interaction.

**Installer flow:**

```
1. Welcome screen — what GPUsion is, one paragraph
2. System check — RAM, CPU generation, Windows version
   → If system check fails, show friendly explanation
   → Minimum: Windows 10 2004, 4GB RAM, AVX2 CPU
3. EULA — MIT license, plain English summary
4. Install location — default C:\Program Files\GPUsion
5. Driver installation — progress bar, no visible console
6. Test signing mode enable — if in dev build, explain why
7. Verification — "GPUsion Virtual Adapter detected ✓"
8. Optional: install Ollama if not present
9. Done — link to getting started guide
```

**Installer technology:** WiX Toolset v4 (preferred) or NSIS

**Silent install support** (for enterprise / IT deployment):

```powershell
gpusion-setup.exe /quiet /norestart
```

---

## 7. Phase 2 — Hardware Dongle Specification

### 7.1 Overview

Phase 2 replaces the CPU inference engine with dedicated hardware. The WDDM virtual driver layer remains identical — only the inference backend changes. Applications see no difference.

### 7.2 POC Hardware Specification

| Parameter | Specification |
|---|---|
| FPGA | Xilinx Artix-7 XC7A100T or Lattice ECP5-85F |
| Interface | USB 3.2 Gen 2 (10Gbps) via USB-C connector |
| Form factor | PCB + enclosure, approximately USB thumb drive × 3 |
| Power | USB bus-powered (max 4.5W at USB 3.x) |
| RAM | 256MB DDR3 on-board for activation buffers |
| Flash | 32MB SPI flash for firmware |
| Operating temp | 0°C – 70°C |

### 7.3 FPGA Architecture

```
┌──────────────────────────────────────────────────────┐
│                  FPGA DIE                            │
│                                                      │
│  ┌─────────────┐    ┌────────────────────────────┐  │
│  │  USB 3.2    │    │    RISC-V Host Core        │  │
│  │  Controller │◄──►│    (VexRiscv or PicoRV32)  │  │
│  │  (soft IP)  │    │    Firmware execution       │  │
│  └─────────────┘    └──────────┬─────────────────┘  │
│                                │                     │
│                                ▼                     │
│                  ┌─────────────────────────┐         │
│                  │   Systolic Array        │         │
│                  │   8×8 MAC units         │         │
│                  │   INT8 matrix multiply  │         │
│                  │   Pipelined, 1 op/cycle │         │
│                  └─────────────────────────┘         │
│                                │                     │
│                                ▼                     │
│                  ┌─────────────────────────┐         │
│                  │   Activation Engine     │         │
│                  │   ReLU, SiLU, GELU      │         │
│                  │   Softmax (approx)      │         │
│                  └─────────────────────────┘         │
│                                │                     │
│                     ┌──────────┴──────────┐          │
│                     │   DDR3 Controller   │          │
│                     │   256MB on-board    │          │
│                     └─────────────────────┘          │
└──────────────────────────────────────────────────────┘
```

### 7.4 Firmware Specification

**Language:** Amaranth HDL (Python-based, open source)  
**RISC-V core:** VexRiscv (lightweight, proven, MIT licensed)  
**Firmware license:** MIT

**Communication protocol (USB host ↔ FPGA):**

```
Host sends:     [OPCODE 1B][TENSOR_DIMS 16B][DATA variable]
FPGA responds:  [STATUS 1B][RESULT_DIMS 16B][DATA variable]

Opcodes:
  0x01  MATMUL       Matrix multiply (core operation)
  0x02  ATTENTION    Attention head computation
  0x03  ACTIVATE     Activation function
  0x04  LAYERNORM    Layer normalisation
  0x05  EMBED        Embedding lookup
  0xFF  PING         Latency measurement
```

### 7.5 Driver Changes for Phase 2

The WDDM virtual driver gains a hardware detection module:

```c
typedef enum {
    GPUSION_BACKEND_CPU   = 0,  // Phase 1: no dongle present
    GPUSION_BACKEND_FPGA  = 1,  // Phase 2: FPGA dongle detected
    GPUSION_BACKEND_ASIC  = 2,  // Phase 3: custom chip detected
} GPUSION_BACKEND;

// On driver load, detect which backend is available
// Fall back gracefully: ASIC → FPGA → CPU
GPUSION_BACKEND gpusion_detect_backend(void);
```

This ensures Phase 1 software installs continue working even when Phase 2 hardware is not present, and automatically accelerate when the dongle is plugged in.

---

## 8. Phase 3 — Custom ASIC Specification

### 8.1 Target Specifications

| Parameter | Target |
|---|---|
| Process node | TSMC 28nm HPC+ |
| Die area | ~25mm² |
| Performance | 4 TOPS (INT8) |
| Power | ≤ 3W (USB bus power compatible) |
| Interface | USB4 / Thunderbolt 3 (40Gbps) |
| On-chip SRAM | 4MB |
| Off-chip memory | LPDDR4X 4GB |
| BOM cost (volume) | Sub-$8 at 100k units |
| MSRP target | ₹1,500 – ₹2,500 |

### 8.2 Architecture

```
RISC-V Host Core (application processor)
    ↓
256-PE Systolic Array (4 TOPS INT8 matrix multiply)
    ↓
Activation Engine (ReLU, SiLU, GELU, Softmax)
    ↓
4MB On-chip SRAM (weight cache + activation buffer)
    ↓
LPDDR4X Controller → 4GB off-chip DRAM
    ↓
USB4 / Thunderbolt 3 Interface (40Gbps)
```

### 8.3 Tape-out Path

| Step | Timeline | Dependency |
|---|---|---|
| RTL design complete | Month 14 | Phase 2 FPGA design validated |
| RTL simulation | Month 15 | RTL complete |
| Synthesis + timing closure | Month 16 | Simulation passing |
| TSMC PDK access | Month 16 | Funding secured |
| GDS-II submission | Month 18 | All sign-offs |
| Silicon return | Month 22 | Post tape-out |
| Bring-up + validation | Month 23–24 | Silicon returned |

### 8.4 Manufacturing Strategy

- **Primary fab:** TSMC 28nm (via authorised shuttle or MPW program)
- **PCB + assembly:** Indian EMS partner (preferred) or Taiwan
- **Enclosure:** Injection moulded, designed in India
- **Certification:** BIS certification for India market, CE/FCC for export

---

## 9. Open Source Strategy

### 9.1 Repository Licensing

| Repository | License | Rationale |
|---|---|---|
| `gpusion-driver` | MIT | Maximum adoption. Core illusion layer must be auditable. |
| `gpusion-inference` | MIT | Encourages contributions from llama.cpp / ONNX community. |
| `gpusion-firmware` | MIT | Released when Phase 2 ships. Signals hardware openness. |
| `gpusion-installer` | MIT | Community can build regional language versions. |
| `gpusion-benchmark` | MIT | Credibility through transparency. |

### 9.2 What Remains Proprietary

Nothing in the software stack is closed. The commercial moat is:

1. **The physical hardware** — the dongle and chip are products, not software
2. **The brand** — GPUsion is a registered trademark
3. **Manufacturing relationships** — supplier and fab partnerships are not IP but are real moats
4. **Support and certification** — enterprise support contracts

### 9.3 Community Milestones

| GitHub Stars | Action |
|---|---|
| 100 | Reach out to Indian tech media (Inc42, YourStory, Analytics India Magazine) |
| 500 | Begin outreach to Qualcomm, MediaTek India partner teams |
| 1,000 | Apply to Y Combinator / Surge / Bharat Innovation Fund |
| 2,500 | Approach Microsoft for official DirectML partnership discussion |
| 5,000 | Series A fundraising with hardware traction data |

### 9.4 Partnership Targets

| Company | Why They Care | Our Approach |
|---|---|---|
| Microsoft | DirectML distribution, Windows AI story | Open contribution to DirectML docs, then direct contact |
| Qualcomm | Snapdragon X Elite AI story needs software | Demonstrate compatibility with Hexagon DSP as backend |
| MediaTek | Dimensity laptop chips entering India market | Driver compatibility work, then partnership pitch |
| Intel | Meteor Lake NPU + Arc GPU both benefit | NPU backend addition to GPUsion inference engine |
| C-DAC | Government AI compute initiative | MoU for deployment in education sector |

---

## 10. Commercial Model

### 10.1 Revenue Streams by Phase

**Phase 1 — No revenue (intentional)**

The software is free. The goal is traction. Attempting to monetise Phase 1 slows adoption and reduces open source credibility.

**Phase 2 — Hardware sales**

| Product | Price | Channel |
|---|---|---|
| GPUsion Stick (FPGA dongle) | ₹2,499 | Amazon India, Flipkart, direct |
| GPUsion Stick — Education | ₹1,799 | Direct to colleges and coaching centres |

**Phase 3 — Scaled hardware + licensing**

| Revenue stream | Model |
|---|---|
| GPUsion Chip (retail) | ₹1,499–₹2,499 consumer dongle |
| GPUsion for Labs | ₹49,999 / 30-device pack for colleges |
| OEM licensing | Per-unit royalty to laptop manufacturers integrating chip |
| Enterprise support | Annual support contract for IT deployments |

### 10.2 Unit Economics (Phase 2 POC target)

| Item | Cost |
|---|---|
| FPGA + PCB BOM | ~₹1,800 |
| Enclosure + assembly | ~₹300 |
| Certification + compliance | ~₹100 amortised |
| **Total COGS** | **~₹2,200** |
| **MSRP** | **₹2,499** |
| **Gross margin** | **~12% (POC)** |

> Phase 3 ASIC drops COGS to ~₹600–800, targeting 60%+ gross margin at scale.

---

## 11. Driver Signing Strategy

This is documented separately because it is a Phase 1 blocker if not planned correctly.

### 11.1 The Problem

Windows 10 and 11 (with Secure Boot enabled) refuse to load kernel-mode drivers that are not signed by a trusted authority. GPUsion is a kernel-mode driver. Unsigned builds will not load on end-user machines.

### 11.2 Development Phase (Months 1–4)

Use Windows test signing mode for all development and internal testing:

```powershell
# Enable (run as Administrator, then restart)
bcdedit /set testsigning on

# Disable when done
bcdedit /set testsigning off
```

Test signing mode is sufficient for all development work. It cannot be used for distribution.

### 11.3 Production Signing Options

**Option A — SignPath Foundation (recommended first attempt)**

SignPath Foundation provides free code signing certificates for genuine open source projects. GPUsion qualifies as MIT-licensed open source.

- Apply at: [signpath.io/foundation](https://signpath.io/foundation)
- Processing time: ~2 weeks
- Cost: ₹0
- Requirement: Public GitHub repository with clear open source license

**Option B — Microsoft WHQL Open Source Program**

Microsoft's Hardware Dev Center countersigns drivers from open source projects through their WHQL (Windows Hardware Quality Labs) program.

- Submit at: [partner.microsoft.com](https://partner.microsoft.com)
- Processing time: 2–6 weeks
- Cost: ₹0 for open source
- Benefit: "Designed for Windows" certification increases user trust

**Option C — Commercial EV Certificate (fallback)**

If Options A and B are rejected or delayed:

| Provider | Annual cost | Notes |
|---|---|---|
| DigiCert | ~₹28,000/yr | Fastest processing |
| Sectigo | ~₹18,000/yr | Lowest cost option |
| GlobalSign | ~₹22,000/yr | Good reputation |

### 11.4 Recommended Timeline

| Month | Signing action |
|---|---|
| 1 | Enable test signing mode for development |
| 2 | Apply to SignPath Foundation when first driver build is functional |
| 3 | Apply to WHQL open source program simultaneously |
| 4 | If neither approved, purchase Sectigo EV certificate |
| 5 | Distribute only signed drivers |

**Budget:** ₹0 (primary path) with ₹18,000–28,000 fallback.

---

## 12. Performance Targets

### 12.1 Phase 1 (Software) Targets

These are targets, not guarantees. Actual performance depends on CPU generation, RAM, and model size.

| Workload | Real RTX 3060 | GPUsion Phase 1 Target | Minimum acceptable |
|---|---|---|---|
| Llama 3 8B INT4 — tokens/sec | ~50 | 6–10 | 3 |
| Llama 3 3B INT4 — tokens/sec | ~100 | 12–18 | 6 |
| Whisper Base — real-time factor | 15× | 0.4× | 0.2× |
| Whisper Tiny — real-time factor | 30× | 0.8× | 0.5× |
| ONNX ResNet-50 — ms/inference | ~2ms | ~80ms | ~200ms |

### 12.2 Phase 2 (FPGA) Targets

| Workload | Phase 1 baseline | Phase 2 target | Improvement |
|---|---|---|---|
| Llama 3 8B INT4 | 6–10 tok/s | 20–30 tok/s | ~3× |
| Whisper Base | 0.4× real-time | 0.9× real-time | ~2× |
| Stable Diffusion 512×512 | Not supported | ~60 sec/img | New capability |

### 12.3 Phase 3 (ASIC) Targets

| Workload | Phase 2 baseline | Phase 3 target |
|---|---|---|
| Llama 3 8B INT4 | 20–30 tok/s | 40–60 tok/s |
| Whisper Base | 0.9× real-time | 2× real-time |
| Stable Diffusion 512×512 | ~60 sec/img | ~15 sec/img |

---

## 13. Roadmap

### Phase 1 — Software Driver

| Milestone | Month | Success Criteria |
|---|---|---|
| Driver skeleton | 1–2 | WDDM virtual adapter visible in Windows Device Manager |
| DirectML passthrough | 2 | DirectML device creation succeeds without error |
| llama.cpp integration | 2–3 | Llama 3 8B model completes single inference via GPUsion |
| Ollama auto-detection | 3 | Ollama selects GPUsion adapter without manual configuration |
| Whisper integration | 3–4 | Whisper Base model transcribes 60-second audio clip |
| Installer v1 | 4 | Non-technical user installs successfully in < 5 minutes |
| Public GitHub launch | 4–5 | Repository public, README complete, 100 stars in first week |
| Benchmark publication | 5–6 | Honest performance data published across 5+ CPU models |

### Phase 2 — Hardware

| Milestone | Month | Success Criteria |
|---|---|---|
| FPGA design complete | 6–7 | Systolic array passes simulation for GEMM operations |
| USB communication | 7–8 | Host–FPGA round-trip latency < 2ms for 1KB payload |
| End-to-end inference | 8–9 | Llama 3 3B completes inference 2× faster than Phase 1 software |
| PCB v1 | 9 | Manufacturable board, USB bus powered, fits in enclosure |
| First 100 units | 10 | Sold and shipped, NPS collected |
| Flipkart / Amazon listing | 11 | Product live on both platforms |
| First 500 units | 12 | Revenue target: ₹12.5 lakhs |

### Phase 3 — ASIC

| Milestone | Month | Success Criteria |
|---|---|---|
| RTL design | 14 | Simulation passing at 4 TOPS INT8 |
| TSMC PDK access | 16 | Funding secured, shuttle slot booked |
| Tape-out | 18 | GDS-II submitted to fab |
| Silicon return | 22 | First samples received |
| Bring-up | 23–24 | All functional blocks validated |
| OEM MOU | 24 | At least one laptop OEM MOU signed |

---

## 14. Risk Register

| ID | Risk | Probability | Impact | Mitigation |
|---|---|---|---|---|
| R01 | WDDM driver signing blocked or delayed | Low | Critical | Apply SignPath + WHQL simultaneously from Month 2. Budget EV cert as fallback. |
| R02 | Performance gap too large for user acceptance | Medium | High | Publish honest benchmarks. Frame as "possible, not fast." Phase 2 closes gap. |
| R03 | CUDA-dependent apps bypass DirectML entirely | High | Medium | Scope clearly: Phase 1 targets DirectML/ONNX apps only. CUDA is Phase 3+ stretch. |
| R04 | Microsoft builds equivalent natively | Low | High | Speed + community moat. Open source forks survive. India-first positioning is distinct. |
| R05 | FPGA BOM cost exceeds Phase 2 margin target | Medium | Medium | Lattice ECP5 alternative. Aggressive enclosure cost reduction. |
| R06 | TSMC access for Phase 3 tape-out | Medium | High | Explore CHIPS India scheme. Consider GlobalFoundries 28nm as alternative. |
| R07 | Competitor open source project launches first | Low | Medium | Speed is the moat. Publish Phase 1 driver skeleton even before feature-complete. |
| R08 | USB bandwidth insufficient for Phase 2 models | Medium | Medium | Limit Phase 2 to quantised models ≤ 7B parameters. USB4 upgrade for production. |

---

## 15. Success Metrics

### Phase 1 — 6 Month Targets

| Metric | Target |
|---|---|
| GitHub stars | 500 |
| Active installs | 100 |
| CPU models tested | 10+ |
| Frameworks supported | 3 (Ollama, llama.cpp, Whisper) |
| Press mentions | 3+ Indian tech publications |
| First-attempt install success rate | > 85% |

### Phase 2 — 12 Month Targets

| Metric | Target |
|---|---|
| Hardware units sold | 500 |
| Revenue | ₹12.5 lakhs |
| NPS | > 40 |
| GitHub stars | 2,000 |
| Partnership conversations | 2+ (Qualcomm / MediaTek / Intel) |

### Phase 3 — 24 Month Targets

| Metric | Target |
|---|---|
| ASIC tape-out complete | Yes |
| Funding secured | ₹5 crore minimum |
| OEM MOU | 1+ signed |
| GitHub stars | 5,000 |
| Monthly active installs | 10,000 |

---

## 16. Glossary

| Term | Definition |
|---|---|
| WDDM | Windows Display Driver Model — Microsoft's driver architecture for display and compute adapters |
| KMD | Kernel Mode Driver — driver code running at the OS kernel level, highest privilege |
| DXGI | DirectX Graphics Infrastructure — Windows API for enumerating graphics adapters |
| DirectML | Direct Machine Learning — Microsoft's GPU-accelerated ML inference API for Windows |
| ONNX | Open Neural Network Exchange — open format for ML models, supported by most frameworks |
| INT4 / INT8 | 4-bit / 8-bit integer quantisation — reduces model size and memory at some accuracy cost |
| FPGA | Field-Programmable Gate Array — reconfigurable hardware chip used for Phase 2 POC |
| ASIC | Application-Specific Integrated Circuit — custom chip designed for one purpose |
| TOPS | Tera Operations Per Second — measure of AI inference compute performance |
| Systolic Array | Hardware architecture for efficient matrix multiplication — used in TPUs, NPUs, and GPUsion |
| VRAM | Video RAM — dedicated memory on a real GPU; GPUsion proxies this into system RAM |
| AVX2 / AVX-512 | Advanced Vector Extensions — Intel/AMD CPU SIMD instruction sets for parallel computation |
| GEMM | General Matrix Multiply — the core mathematical operation in neural network inference |
| RISC-V | Open-source CPU instruction set architecture used in GPUsion FPGA/ASIC host core |
| EV Certificate | Extended Validation code signing certificate required for Windows kernel driver distribution |
| WHQL | Windows Hardware Quality Labs — Microsoft's driver certification programme |
| BIS | Bureau of Indian Standards — mandatory certification for electronics sold in India |
| CHIPS India | Government of India semiconductor incentive scheme under the India Semiconductor Mission |

---

*GPUsion PRD v1.0 — May 2026*  
*MIT Licensed — Built in India 🇮🇳*
