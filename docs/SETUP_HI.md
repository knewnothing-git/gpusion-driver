# GPUsion सेटअप गाइड (हिंदी)

यह गाइड उन डेवलपर्स के लिए है जो GPUsion के मौजूदा सोर्स कोड को देखना, बदलना और सत्यापित करना चाहते हैं।

> **महत्वपूर्ण:** GPUsion अभी शुरुआती विकास चरण में है। इस repository में अभी production-ready Windows driver installer या पूर्ण WDK build pipeline उपलब्ध नहीं है। वर्तमान में repository में जो build लगातार CI में सत्यापित होता है, वह Linux पर CMake/GCC के साथ driver source का **syntax-check build** है।

## अभी क्या काम करता है?

वर्तमान repository में:

- WDDM/KMDF driver source code मौजूद है।
- Linux compatibility stubs की मदद से C source files compile करके syntax/type errors पकड़े जाते हैं।
- GitHub Actions में GCC syntax check और `cppcheck` static analysis चलती है।
- Windows WDK build workflow अभी future work के रूप में CI file में commented है।
- `build.ps1`, `install.ps1` या एक production driver installer अभी repository में मौजूद नहीं हैं।

इसलिए नीचे दिए गए commands वही हैं जिन्हें वर्तमान repository वास्तव में support करती है।

## 1. Repository clone करें

```bash
git clone https://github.com/knewnothing-git/gpusion-driver.git
cd gpusion-driver
```

अगर आप contribution करना चाहते हैं, तो पहले GitHub पर repository fork करें और अपना fork clone करें:

```bash
git clone https://github.com/YOUR_USERNAME/gpusion-driver.git
cd gpusion-driver
git remote add upstream https://github.com/knewnothing-git/gpusion-driver.git
```

## 2. Linux syntax-check के लिए prerequisites

Ubuntu/Debian पर:

```bash
sudo apt-get update
sudo apt-get install -y cmake gcc g++ ninja-build cppcheck
```

यह build Windows driver binary नहीं बनाता। इसका उद्देश्य driver के platform-independent C code को compile करके syntax errors, type mismatches और warnings पकड़ना है।

## 3. Driver source का syntax-check build चलाएँ

Repository root से:

```bash
cmake -B build -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build --parallel
```

सफल होने पर `gpusion_syntax_check` static library build होगी। यह **functional Windows GPU driver नहीं** है; यह वही validation path है जिसे project का Linux CI उपयोग करता है।

## 4. Static analysis चलाएँ

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

अगर command exit code `0` के साथ समाप्त होती है, तो configured cppcheck rules ने कोई blocking issue नहीं पाया।

## 5. मुख्य source directories

मौजूदा driver code मुख्य रूप से इन paths में है:

```text
driver/
├── compat/        # Linux CI के लिए Windows API compatibility stubs
├── dxgi/          # DXGI adapter enumeration
├── kmdf/          # KMDF / driver entry logic
├── vram/          # Virtual VRAM logic
├── wddm/          # WDDM miniport-related code
├── gpusion.h
└── gpusion.inf
```

Root का `CMakeLists.txt` बताता है कि Linux syntax-check में कौन-कौन सी C files compile होती हैं।

## 6. Windows + WDK की वर्तमान स्थिति

GPUsion का अंतिम लक्ष्य Windows पर WDDM virtual GPU driver है, लेकिन repository में अभी पूर्ण Windows build project और installer उपलब्ध नहीं हैं। `.github/workflows/ci.yml` में भविष्य के Windows WDK build का outline मौजूद है, लेकिन वह अभी disabled/commented है।

अर्थात इस समय:

- Visual Studio + WDK install करने भर से repository में documented one-command driver build उपलब्ध नहीं है।
- किसी ऐसे `scripts/build.ps1` या `scripts/install.ps1` को चलाने की कोशिश न करें जो repository में मौजूद नहीं है।
- Windows build pipeline जोड़ते समय उसकी commands, project files और prerequisites को उसी PR में दस्तावेज़ित किया जाना चाहिए।

## 7. बदलाव करने का सुरक्षित workflow

```bash
git checkout -b feature/my-change
```

बदलाव के बाद कम-से-कम मौजूदा validation चलाएँ:

```bash
cmake -B build -G Ninja \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

और static analysis:

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

फिर commit और push करें:

```bash
git add -A
git commit -m "feat: describe your change"
git push -u origin feature/my-change
```

इसके बाद upstream repository में Pull Request खोलें।

## 8. सामान्य समस्याएँ

### `cmake: command not found`

CMake install करें:

```bash
sudo apt-get install cmake
```

### Ninja generator नहीं मिलता

```bash
sudo apt-get install ninja-build
```

### Windows headers से संबंधित compile error

Linux CI build को `driver/compat` stubs के माध्यम से Windows APIs emulate करने के लिए बनाया गया है। अगर आपने नई Windows API जोड़ी है, तो syntax-check को support करने के लिए compatibility stub भी जोड़ना पड़ सकता है।

### क्या इस build से Device Manager में GPUsion दिखाई देगा?

नहीं। Linux syntax-check केवल source validation है। Device Manager में adapter दिखाई देने के लिए पूर्ण Windows WDK driver build, signing और installation path की आवश्यकता होगी, जो अभी project में पूरा नहीं है।

## 9. योगदान करते समय क्या ध्यान रखें

- Documentation में केवल वही command लिखें जो repository में वास्तव में मौजूद और चलने योग्य हो।
- नई build script जोड़ते समय उसके prerequisites और expected output भी document करें।
- Driver behavior के दावे को source/tests से verify करें।
- Kernel/WDK changes के लिए Windows पर वास्तविक परीक्षण आवश्यक होगा; Linux syntax-check उसका विकल्प नहीं है।

## आगे क्या पढ़ें?

- [`README.md`](../README.md) — project overview
- [`CONTRIBUTING.md`](../CONTRIBUTING.md) — contribution rules और bounty information
- [`CMakeLists.txt`](../CMakeLists.txt) — current Linux syntax-check configuration
- [`.github/workflows/ci.yml`](../.github/workflows/ci.yml) — CI में चलने वाले current checks

---

यह गाइड जानबूझकर GPUsion की **मौजूदा** स्थिति को document करती है, न कि roadmap में प्रस्तावित भविष्य की build pipeline को। जैसे-जैसे Windows WDK build और installer वास्तव में repository में जुड़ेंगे, इस दस्तावेज़ को भी उनके साथ update किया जाना चाहिए।
