# GPUsion सेटअप गाइड (हिंदी)

यह गाइड उन हिंदी-भाषी contributors के लिए है जो GPUsion repository को अपने सिस्टम पर तैयार करके source code देखना, documentation बदलना या driver development शुरू करना चाहते हैं।

> **स्थिति:** GPUsion अभी **Early Development** में है। यह production-ready driver नहीं है। मौजूदा repository में Windows driver को build/install करने के लिए तैयार `scripts/build.ps1`, `scripts/install.ps1` या installer मौजूद नहीं हैं। इसलिए नीचे केवल वही steps दिए गए हैं जो अभी repository में वास्तव में उपलब्ध हैं।

## 1. अभी क्या काम करता है?

मौजूदा source tree में WDDM/KMDF driver skeleton और Linux CI के लिए syntax-check configuration मौजूद है। मुख्य folders हैं:

```text
driver/
├── compat/     # Linux syntax-check के लिए Windows API stubs
├── dxgi/       # DXGI adapter enumeration
├── kmdf/       # KMDF driver entry / backend detection
├── vram/       # Virtual VRAM logic
└── wddm/       # WDDM miniport paths
```

`CMakeLists.txt` का उपयोग **केवल Linux/GCC syntax check** के लिए होता है। यह वास्तविक Windows driver binary नहीं बनाता। वास्तविक Windows kernel-driver development के लिए Windows Driver Kit (WDK) और Visual Studio/MSBuild की आवश्यकता होगी।

## 2. ज़रूरी software

### सामान्य contribution के लिए

- Git
- GitHub account
- कोई text editor या IDE (उदाहरण: Visual Studio Code)

### Windows driver development के लिए

- Windows 10 या Windows 11 (64-bit)
- Visual Studio 2022 का **Desktop development with C++** workload
- Windows Driver Kit (WDK)
- Git for Windows
- Python 3.10+ (tooling/benchmarks के लिए उपयोगी)

WDK की official installation guide:
https://learn.microsoft.com/windows-hardware/drivers/download-the-wdk

## 3. Repository fork और clone करें

पहले GitHub पर `knewnothing-git/gpusion-driver` को fork करें। फिर अपने fork को clone करें:

```bash
git clone https://github.com/YOUR_USERNAME/gpusion-driver.git
cd gpusion-driver
```

Original repository को `upstream` remote के रूप में जोड़ें:

```bash
git remote add upstream https://github.com/knewnothing-git/gpusion-driver.git
git remote -v
```

नई contribution के लिए अलग branch बनाएँ:

```bash
git switch -c docs/my-change
```

या पुराने Git versions पर:

```bash
git checkout -b docs/my-change
```

## 4. Source tree को verify करें

Clone के बाद कम-से-कम ये files/folders दिखने चाहिए:

```text
.github/
docs/
driver/
CMakeLists.txt
CONTRIBUTING.md
LICENSE
README.md
```

यदि आप किसी command को documentation से copy कर रहे हैं, पहले यह देख लें कि referenced script/file repository में वास्तव में मौजूद है। Project तेजी से बदल रहा है और कुछ roadmap documentation future files का उल्लेख कर सकती है।

## 5. Local syntax check (Linux, WSL या Linux VM)

Project की वर्तमान CI driver C files को GCC से compile करके syntax/type errors पकड़ती है। Windows पर इसे WSL2/Ubuntu में चलाया जा सकता है।

Ubuntu/WSL में dependencies install करें:

```bash
sudo apt-get update
sudo apt-get install -y cmake gcc g++ ninja-build cppcheck
```

Repository root से configure और build करें:

```bash
cmake -B build -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build --parallel
```

> **Current-main note (commit `ebdbd2473456fcff033baa904ac3fe1f2e6e105b`):** ऊपर का syntax build अभी existing diagnostic-format bug के कारण `driver/wddm/add_device.c` और `driver/vram/vram_proxy.c` में `-Werror=format-extra-args` / format-type errors पर रुक सकता है। यह setup की गलती नहीं है; इसे existing issue [#7](https://github.com/knewnothing-git/gpusion-driver/issues/7) में reproduce और document किया जा चुका है। नया duplicate issue न खोलें।

Static analysis चलाने के लिए:

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

> यह सिर्फ source validation है। इससे install करने योग्य Windows `.sys` driver नहीं बनता।

## 6. Windows driver development और Test Signing

Windows kernel driver development में unsigned/test-signed driver load करने के लिए test-signing mode की आवश्यकता पड़ सकती है। Administrator PowerShell में:

```powershell
bcdedit /set testsigning on
```

फिर Windows restart करें। काम पूरा होने पर mode बंद करने के लिए:

```powershell
bcdedit /set testsigning off
```

और दोबारा restart करें।

> ⚠️ **सुरक्षा चेतावनी:** Test Signing Windows की driver-signature policy को development के लिए ढीला करता है। इसे केवल development/test मशीन पर सक्षम करें। किसी अनजान या untrusted driver को install न करें। Test-signed development build को सामान्य users में distribute न करें।

### मौजूदा repository की सीमा

इस समय repository में documented `scripts/build.ps1`, `scripts/install.ps1`, `scripts/test.ps1` और Windows project/build files का पूरा working path उपलब्ध नहीं है। इसलिए इस गाइड में कोई ऐसा Windows build/install command नहीं दिया गया है जिसे वर्तमान tree पर सत्यापित नहीं किया जा सकता।

जब Windows WDK build path repository में जोड़ा जाए, setup guide को उसी release/commit के verified commands के साथ update किया जाना चाहिए।

## 7. Contribution भेजने से पहले

Project की contribution policy के अनुसार:

1. Existing issues और pull requests search करें ताकि duplicate काम न हो।
2. जिस change पर काम कर रहे हैं उसके issue में `I'm working on this` लिखें, या नया focused issue खोलें।
3. अपने fork में अलग branch पर बदलाव करें।
4. उपलब्ध checks चलाएँ और output verify करें।
5. Clear commit message लिखें।
6. Pull request में related issue को link करें, जैसे `Closes #123`।

उदाहरण commit message:

```text
docs: add Hindi developer setup guide
```

## 8. Upstream के बदलाव अपने fork में लाएँ

काम लंबा चलने पर upstream से latest changes लें:

```bash
git fetch upstream
git switch main
git merge --ff-only upstream/main
```

फिर अपनी branch को latest `main` पर rebase करना चाहें तो:

```bash
git switch docs/my-change
git rebase main
```

Conflict आने पर files ध्यान से review करें; blind conflict resolution न करें।

## 9. मदद चाहिए तो क्या जानकारी दें?

Issue खोलते समय जितना संभव हो उतना reproducible context दें:

- Windows version (`winver`)
- CPU model
- RAM
- Git commit SHA (`git rev-parse HEAD`)
- Exact command
- पूरा error message
- Expected result
- Actual result

इससे maintainer को समस्या reproduce करने में आसानी होगी।

## 10. महत्वपूर्ण links

- Repository: https://github.com/knewnothing-git/gpusion-driver
- Contribution rules: [`../CONTRIBUTING.md`](../CONTRIBUTING.md)
- Product/architecture document: [`PRD_v1.0.md`](PRD_v1.0.md)
- Windows Driver Kit: https://learn.microsoft.com/windows-hardware/drivers/download-the-wdk

---

**नोट:** GPUsion का लक्ष्य बड़ा है, लेकिन development setup में सबसे महत्वपूर्ण चीज़ reproducibility और honesty है। जिस build, script या feature को आपने खुद verify नहीं किया है, उसे working न लिखें।
