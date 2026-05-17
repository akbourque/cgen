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
Make sure the generated sub-commands `cgen-vec`, `cgen-sbovec` and `cgen-map` are in a directory that is in your PATH so that cgen can find them.

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
## Generated Code Documentation & Tooltips
Every single struct and method emitted by cgen is fully wrapped in struct Javadoc-compliant Doxygen comment configurations.
When imported into modern development environments (like VS Code,CLion, or Neovim), your language server (LSP) will parse
the metadata definitions natively, providing beautiful hover information, functional parameter signatures, and
autocomplete context definitions as you type.

## Extending cgen: Adding Custom Sub-commands

`cgen` is built on a decoupled, highly extensible framework architecture. You can easily add your own domain-specific data structures or template generators without writing any argument-parsing, flag-validation, or file-handling boilerplate.

### Step 1: Choose Your Engine Type
Determine if your custom generator requires a single data type (like a vector) or a dual-type pair (like a map or result).

### Step 2: Implement Your Sub-command File
Create a new standalone source file (e.g., `cgen-stack.c`). Define your code layout templates using string variables, leveraging our standard token wrappers:
* For Single-Type Engines: Use `{{00}}` (exact type name) and `{{00BU}}` (screaming uppercase base type name).
* For Dual-Type Engines: Use `{{KEY}}`/`{{VAL}}` and their associated formatting modifiers (`_B`, `_BU`).

### Step 3: Wire into the Framework Entrypoint
Populate the appropriate app configuration structure and forward `argc`/`argv` straight into the framework's optimized runner paths:

```c
    #include "cgen_framework.h"

    const char *STACK_H = "/* Your header template containing {{00}} tokens */";
    const char *STACK_C = "/* Your source template containing {{00}} tokens */";

    int main(int argc, char **argv) {
        cgen_app_def_t app = {
            .subcommand_name = "stack",
            .opt_spec        = "=ttypename",
            .template_h      = STACK_H,
            .template_c      = STACK_C
        };
        // Let the framework handle help text, out-dirs, token shifts, and file I/O
        return cgen_app_run(&app, argc - 1, argv + 1);
    }
``` 
### Step 4: Update the Build System
Open the Makefile, add your new executable name to the all and clean target variables,
and supply the single-line framework compilation target:
```Makefile
    cgen-stack: cgen-stack.o $(FRAMEWORK_$OBJS) $(VENDOR_OBJS)
            $(CC) $(CFLAGS) $^ -o $@
```

## More subcommand engines to come
Coming soon more subcommand engines to generate other usable containers and types.


## Acknowledgments

* **Gemini (Google)** - Acted as an AI pair programmer for porting logic,
structural code lowering, and expanding test coverage.
