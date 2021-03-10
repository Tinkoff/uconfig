# uconfig

[![Language C++](https://img.shields.io/badge/language-c++-blue.svg)](https://isocpp.org)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

C++ header-only library to parse and emit **multi-format** configuration for your app. For example, you can parse JSON-file into the config and then compliment it from environment variables.

Supported types:
* plain variables (`int`, `double`, `std::string` etc);
* vectors of variables or configs;
* nested configs.

Supported formats:
* [Rapidjson](https://rapidjson.org/) – parse and emit `rapidjson::Value`
* Env – parse values from environment, emit to `std::map<std::string, std::string>`

Library allows to implement [custom formats](#custom-formats) as well as [custom types](#custom-types).

## Quickstart

1. Derive classes from `uconfig::Config` with needed format.
2. Define all variables in it. Plain variables should be wrapped in `uconfig::Variable`, lists – `uconfig::Vector`, nested configs should be derived from `uconfig::Config`
3. Overload `Init()` where call `Register()` for all variables with respected paths/names

```c++
// Derive struct from uconfig::Config<uconfig::EnvFormat>
struct LogConfig: public uconfig::Config<uconfig::EnvFormat>
{
    // Define variables
    uconfig::Variable<std::string> file; // mandatory string
    uconfig::Variable<unsigned> rotate_mb{1024}; // optional unsigned with default = 1024

    using uconfig::Config<uconfig::EnvFormat>::Config;

    // Define naming scheme for variables
    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_FILE", &file);
        Register<uconfig::EnvFormat>(config_path + "_ROTATE_MB", &rotate_mb);
    }
};

struct NodeConfig: public uconfig::Config<uconfig::EnvFormat>
{
    uconfig::Variable<std::string> host; // mandatory string
    uconfig::Variable<unsigned> port; // mandatory unsigned

    using uconfig::Config<uconfig::EnvFormat>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_HOST", &int_var);
        Register<uconfig::EnvFormat>(config_path + "_PORT", &optional_int_var);
    }
};

struct AppConfig: public uconfig::Config<uconfig::EnvFormat>
{
    LogConfig log_config; // mandatory LogConfig
    uconfig::Variable<unsigned> timeout_ms{100}; // optional unsigned with default = 100
    uconfig::Vector<NodeConfig> nodes; // mandatory vector of NodeConfig
    uconfig::Vector<std::string> endpoints; // mandatory vector of strings

    using uconfig::Config<uconfig::EnvFormat>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_LOG", &log_config);
        Register<uconfig::EnvFormat>(config_path + "_TIMEOUT_MS", &timeout_ms);
        Register<uconfig::EnvFormat>(config_path + "_NODE", &nodes);
        Register<uconfig::EnvFormat>(config_path + "_ENDPOINT", &endpoints);
    }
};
```

Now you can call `Parse()` with proper format instance passed into it:
```c++
AppConfig app_config;

uconfig::EnvFormat formatter;
// "APP" used as prefix for env names to avoid clashes
// nullptr as a source because uconfig::EnvFormat access variables via getenv()
app_config.Parse(formatter, "APP", nullptr);
```

Will parse:
* `app_config.log_config.file` from `APP_LOG_FILE`
* `app_config.log_config.rotate_mb` from `APP_LOG_ROTATE_MB`
* `app_config.timeout_ms` from `APP_TIMEOUT_MS`
* `app_config.nodes[0].host` from `APP_TIMEOUT_NODE_0_HOST`, [1] from `...NODE_1...` etc
* `app_config.nodes[0].port` from `APP_TIMEOUT_NODE_0_PORT`, [1] from `...NODE_1...` etc
* `app_config.endpoints[0]` from `APP_ENDPOINT_0`, [1] from `...ENDPOINT_1...` etc

### Important note
JSON formatter uses [JSON-pointer](https://tools.ietf.org/html/rfc6901) as names identifier, therefore use `'/'` to delimit nested variables instead of `'_'` for env.

## Detailed description

### Nested variable names

Full name for the variable formed by nested calls of `void Config<>::Init(const std::string& config_path)` with parent name passed as `config_path`.
By design variables naming scheme is arbitrary, meaning you are free to call `Register()` with whatever string you want. But if format supports hierarchy, names should respect it and represent some hierarchical paths.

For example, JSON-objects have hierarchy, so `uconfig::RapidjsonFormat` uses JSON-pointer for variable names, so
```c++
Register<uconfig::RapidjsonFormat>("/a/b/c", &variable);
```
would tell to lookup for `variable` as the member "c" of the object "b" of the object "a":
```
{
    "a" : {
        "b" : {
            "c" : 123
        }
    }
}
```

This allows to have **single global config** and not lose its' hierarchy for supported formats:
```c++
struct GlobalConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> log_file;
    uconfig::Variable<unsigned> log_rotate_mb;
    uconfig::Variable<std::string> upsteam_url;
    uconfig::Variable<unsigned> upsteam_timeout_ms{100};
    uconfig::Variable<std::string> server_host;
    uconfig::Variable<unsigned> server_port;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string&) override
    {
        // register with names
        Register<uconfig::EnvFormat>("CFG_LOG_FILE", &log_file);
        Register<uconfig::EnvFormat>("CFG_LOG_ROTATE_MB", &log_rotate_mb);
        Register<uconfig::EnvFormat>("CFG_UPSTEAM_URL", &upsteam_url);
        Register<uconfig::EnvFormat>("CFG_UPSTEAM_TIMEOUT_MS", &upsteam_timeout_ms);
        Register<uconfig::EnvFormat>("CFG_SERVER_HOST", &server_host);
        Register<uconfig::EnvFormat>("CFG_SERVER_PORT", &server_port);
        // register with json-pointers
        Register<uconfig::RapidjsonFormat<>>("/log/file", &log_file);
        Register<uconfig::RapidjsonFormat<>>("/log/rotate_mb", &log_rotate_mb);
        Register<uconfig::RapidjsonFormat<>>("/upsteam/url", &upsteam_url);
        Register<uconfig::RapidjsonFormat<>>("/upsteam/timeout_ms", &upsteam_timeout_ms);
        Register<uconfig::RapidjsonFormat<>>("/server/host", &server_host);
        Register<uconfig::RapidjsonFormat<>>("/server/port", &server_port);
    }
};
```

Or to make **separate configs** and include them into `GlobalConfig`:

```c++
struct LogConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> file;
    uconfig::Variable<unsigned> rotate_mb;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_FILE", &file);
        Register<uconfig::EnvFormat>(config_path + "_ROTATE_MB", &rotate_mb);

        Register<uconfig::RapidjsonFormat<>>(config_path + "/file", &file);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/rotate_mb", &rotate_mb);
    }
};

struct UpstreamConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> url;
    uconfig::Variable<unsigned> timeout_ms{100};

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_URL", &upsteam_url);
        Register<uconfig::EnvFormat>(config_path + "_TIMEOUT_MS", &upsteam_timeout_ms);

        Register<uconfig::RapidjsonFormat<>>(config_path + "/url", &upsteam_url);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/timeout_ms", &upsteam_timeout_ms);
    }
};

struct ServerConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> server_host;
    uconfig::Variable<unsigned> server_port;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_HOST", &server_host);
        Register<uconfig::EnvFormat>(config_path + "_PORT", &server_port);

        Register<uconfig::RapidjsonFormat<>>(config_path + "/host", &server_host);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/port", &server_port);
    }
};

struct GlobalConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    LogConfig log_config;
    UpstreamConfig upstream_config;
    ServerConfig server_config;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string&) override
    {
        Register<uconfig::EnvFormat>("CFG_LOG", &log_config);
        Register<uconfig::EnvFormat>("CFG_UPSTEAM", &upstream_config);
        Register<uconfig::EnvFormat>("CFG_SERVER", &server_config);

        Register<uconfig::RapidjsonFormat<>>("/log", &log_config);
        Register<uconfig::RapidjsonFormat<>>("/upsteam", &upstream_config);
        Register<uconfig::RapidjsonFormat<>>("/server", &server_config);
    }
};
```

### Optional variables

Any configuration variable can be declared as optional, for different types it has different meaning

#### Optional `uconfig::Variable`

### Multiformat configuration
### Custom formats
### Custom types



## How to build

This library supposed to be somewhat multi-platform, however, it was tested and mainly used on ubuntu and macOS. Therefore build instructions are given for only these OSes.

### Ubuntu dependencies

```bash
sudo apt update
sudo apt install build-essential cmake
```

### macOS dependencies

```bash
brew install cmake pkg-config
```

### Build

Prefer [out-of-source](https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#what-is-an-out-of-source-build) building:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
make install
```

You can install it (sudo may be required):
```bash
make install
```

Or test:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..
make test
```

### Cmake options

* **CMAKE_BUILD_TYPE** - [build type](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html). `RelWithDebInfo` by default.
* **BUILD_SHARED_LIBS** - [build shared or static library](https://cmake.org/cmake/help/v3.0/variable/BUILD_SHARED_LIBS.html). `ON` by default.
* **BUILD_TESTING** - [build tests or not](https://cmake.org/cmake/help/latest/module/CTest.html). `OFF` by default.
* **URITEMPLATE_BUNDLED_MODE** - if the library is being built as a part of another project. If this options is set then *BUILD_SHARED_LIBS* forces to be OFF. Defined by `"${PROJECT_SOURCE_DIR}" == "${CMAKE_SOURCE_DIR}"` by default.

## License

Developed at **Tinkoff.ru** in 2021.\
Distibuted under **Apache License 2.0** [LICENSE](./LICENSE). You may also obtain this license at https://www.apache.org/licenses/LICENSE-2.0.

## Contacts

Author - i.s.vovk@tinkoff.ru\
Current maintainer - i.s.vovk@tinkoff.ru
