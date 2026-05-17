# cgen 🚀

A Cargo-inspired type-safe container and code generation utility for C11. 

`cgen` eliminates the classic dilemma of C container design. Instead of forcing
you to choose between type-erased `void *` structures (which destroy CPU cache
locality and require manual pointer casting) or unreadable preprocessor macro
clouds, `cgen` acts as a compile-time code generator. It spits out pristine, flat,
type-safe, and fully documented data structures tailored exactly to your specified types.

---

## 🛠 Features

* **`cgen vec <type>`**: Emits dynamic, heap-allocated vectors featuring geometric capacity doubling boundaries.
* **`cgen sbovec <type>`**: Emits advanced Small-Buffer Optimized vectors that can borrow arbitrary local stack buffers for zero-allocation fast paths, transparently escalating to the heap only if boundaries overflow.
* **`cgen map <key_type> <val_type>`**: Emits high-performance open-addressed hash maps inspired by Google's SwissTable architecture, utilizing an 8-bucket SWAR (SIMD Within A Register) parallel control array to scan metadata in a single CPU instruction loop.

---

## 🏗 Architecture Layout

`cgen` utilizes an extensible driver-proxy model to manage subcommands cleanly:

1. **The Dispatcher (`main.c`)**: A lightweight traffic-cop executable. When you type `cgen vec int`, it hooks the arguments and attempts to execute a sub-binary named `cgen-vec` locally or via your system PATH context.
2. **The Subcommand Engines (`cgen-vec.c`, `cgen-sbovec.c`, `cgen-map.c`)**: Independent, type-specific generation components containing raw source code templates.
3. **The Token Framework (`cgen_framework.c`)**: A shared pipeline utility that automatically processes token variations and clobbers formatted files cleanly to disk.

---

## 🚀 Quick Start & Compilation

### 1. Build the Toolchain
Compile the entire suite of generators from the source root directory using the central configuration engine:
```bash
make
```
Make sure the generated sub-commands `cgen-vec`, `cgen-sbovec` and `cgen-map` are in a directory that is in your PATH.

### 2. Generate Type-Safe Containers
Run the unified driver executable to spin up containers for whatever navtive or custom types your application 
demands:

```bash
 - Generate a standard integer vector
./cgen vec int

 - Generate an SBO vector optimized for floating-point calculations
./cgen sbovec float

 - Generate a high-speed SwissTable mapping chars to doubles
./cgen map char double
```
### 3. Global Directory Routing
Both the vec and sbovec subcommand structures suport target directory routing rules.
Use the -o or --out-dir options to point outputs directly to your source folders:
```bash
    ./cgen vec int -o ./src/containers
```
## 📝 Generated Code Documentation & Tooltips
Every single struct and method emitted by cgen is fully wrapped in struct Javadoc-compliant Doxygen comment configurations.
When imported into modern development environments (like VS Code,CLion, or Neovim), your language server (LSP) will parse
the metadata definitions natively, providing beautiful hover information, functional parameter signatures, and
autocomplete context definitions as you type.


## Acknowledgments

* **Gemini (Google)** - Acted as an AI pair programmer for porting logic,
structural code lowering, and expanding test coverage.
