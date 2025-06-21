# AnalysisPipelineStages

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

**AnalysisPipelineStages** is a modular C++ shared library providing reusable pipeline stages for data analysis workflows built on top of ROOT and popular header-only libraries like `spdlog` and `nlohmann_json`. It is designed to be easily integrated into your own analysis framework, either as a standalone library or as a submodule.

---

## Features

* Modern C++17 codebase
* ROOT integration with dictionary generation
* Publicly exposes dependencies with proper transitive linking
* Easy inclusion via CMake `find_package` or as a git submodule
* Supports seamless installation with CMake export targets
* Position-independent code and shared library versioning
* Generates `compile_commands.json` for IDE tooling

---

## Requirements

* C++17 compatible compiler (GCC 7+, Clang 6+, MSVC 2017+)
* [ROOT](https://root.cern) (Core, RIO, Tree components)
* CMake 3.14 or newer
* Git (for submodules)
* [spdlog](https://github.com/gabime/spdlog) (version 1.13.0 or newer)
* [nlohmann/json](https://github.com/nlohmann/json) (version 3.11.3 or newer)


---

## Quick Start

### Standalone Build & Install

```bash
git clone https://github.com/jaca230/analysis_pipeline_stages
cd analysis_pipeline_stages
./scripts/build.sh
```

This will:

* Build the shared library with ROOT dictionary support
* Fetch header-only dependencies automatically

to install:
```bash
./scripts/install.sh
```

---

## Using AnalysisPipelineStages

### As an Installed Package (via `find_package`)

In your project's `CMakeLists.txt`:

```cmake
find_package(AnalysisPipelineStages REQUIRED)

add_executable(my_app src/main.cpp)

target_link_libraries(my_app PRIVATE AnalysisPipelineStages::analysis_pipeline_stages)
```

CMake will locate the installed package and handle all include directories and transitive dependencies (ROOT, spdlog, nlohmann\_json).

---

### As a Git Submodule

Add as a submodule in your repository:

```bash
git submodule add https://github.com/jaca230/analysis_pipeline_stages external/AnalysisPipelineStages
git submodule update --init --recursive
```

In your project's `CMakeLists.txt`:

```cmake
add_subdirectory(external/AnalysisPipelineStages)

add_executable(my_app src/main.cpp)

target_link_libraries(my_app PRIVATE AnalysisPipelineStages::analysis_pipeline_stages)
```

The submodule's dependencies (`spdlog` and `nlohmann_json`) will be fetched automatically by the submodule’s CMake and exported transitively.

---

## Development

* Run `cmake` with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` to generate `compile_commands.json` for IDEs like VSCode or CLion.
* Library source lives under `src/`, public headers under `include/stages/`.
* ROOT dictionary headers and sources auto-generated during build.

---

## 🔌 Adding a New Stage

To define a new processing stage in the analysis pipeline:

### 1. **Create a New Class Derived from `BaseStage`**

Create a new header/source pair in the `include/stages/` and `src/` directories respectively.

#### Example: `include/stages/my_stage.h`

```cpp
#ifndef ANALYSIS_PIPELINE_STAGES_MYSTAGE_H
#define ANALYSIS_PIPELINE_STAGES_MYSTAGE_H

#include "stages/base_stage.h"

class MyStage : public BaseStage {
public:
    MyStage();
    ~MyStage() override;

    void Process() override;
    std::string Name() const override;

protected:
    void OnInit() override; // Use this to initialize from `parameters_`
};

#endif // ANALYSIS_PIPELINE_STAGES_MYSTAGE_H
```

#### Example: `src/my_stage.cpp`

```cpp
#include "stages/my_stage.h"
#include <spdlog/spdlog.h>

MyStage::MyStage() = default;
MyStage::~MyStage() = default;

void MyStage::OnInit() {
    spdlog::info("[{}] Initialized with config: {}", Name(), parameters_.dump());
}

void MyStage::Process() {
    spdlog::info("[{}] Processing data...", Name());
    // Do something with tree_ using SafeTreeAccess(...)
}

std::string MyStage::Name() const {
    return "MyStage";
}

ClassImp(MyStage)
```

---

### 2. **Register the Class in `LinkDef.h`**

Edit `include/stages/LinkDef.h` to include your new class:

```cpp
#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class BaseStage+;
#pragma link C++ class DummyStage+;
#pragma link C++ class MyStage+;  // 👈 Add this line

#endif
```

> ✅ This ensures ROOT can generate the dictionary needed for dynamic class instantiation via `TClass::New()`.

---

### 3. **Rebuild the Library**

Run the build script to regenerate the ROOT dictionary and recompile:

```bash
./scripts/build.sh
```

If you're installing the project system-wide:

```bash
./scripts/install.sh
```

---

### 4. **Use the Stage in Your Framework**

Once compiled, the new stage can be loaded dynamically from the library (e.g., using `TClass::GetClass("MyStage")->New()`) or linked statically.

Make sure your runtime config (if using one) references the correct class name as returned by `Name()`.


## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---


Thanks — that clarification helps a lot. Here's an updated **TODO** section that better reflects your intended design:

---

## TODO

### 🧩 Pluggable Serializer System for `PipelineDataProduct`

Implement a custom serialization mechanism that allows efficient and flexible control over how `TObject`s (like `TH1*`) are serialized in the pipeline.

#### 🔧 Design Overview

* **Serializer is assigned per data product** at creation time, based on stage config:

  ```json
  {
    "product_name": "my_hist",
    "serializer": "th1_bins_only"
  }
  ```

* A global **Serializer Registry** will hold all known serializers, keyed by string names like `"th1_bins_only"`, `"th1_summary"`, etc.

* Each `PipelineDataProduct` optionally stores the name of its serializer (`std::string serializerId_`).

* `PipelineDataProduct::serializeToJson()` will:

  1. Look up the serializer by name in the registry.
  2. If found, call it and return the resulting JSON.
  3. If not found or unset, fall back to ROOT's `TBufferJSON::ConvertToJSON()`.

#### 🗂 Example Serializer Functions

| Name            | Description                                 |
| --------------- | ------------------------------------------- |
| `th1_bins_only` | Serialize only bin contents (compact, fast) |
| `th1_summary`   | Add entries, mean, RMS                      |
| `tgraph_basic`  | Serialize X/Y points of a `TGraph`          |
| `default`       | Fall back to ROOT `TBufferJSON`             |
| `none`          | Nothing serialized, for intermediate stages |

#### 📦 Serializer Registry API (planned)

```cpp
using SerializerFn = std::function<nlohmann::json(const TObject*)>;

class SerializerRegistry {
public:
    static SerializerRegistry& instance();
    void registerSerializer(const std::string& name, SerializerFn fn);
    std::optional<SerializerFn> get(const std::string& name) const;
};
```

#### 📌 Where to register

* A central `serializer_library.cpp` should call `registerSerializer(...)` for each known one.
* Stages like `TH1BuilderStage` can simply set `serializerId_` during product creation.

#### ✅ Benefits

* Full control over data volume for high-rate DQM.
* Easy to swap between fast/compact or full serialization without code changes.
* No duplication of logic across pipeline stages.

---

Let me know if you'd like me to turn this into an actual implementation stub or break it out into smaller steps!

