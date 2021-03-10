# uconfig

[![Language C++](https://img.shields.io/badge/language-c++-blue.svg)](https://isocpp.org)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

C++ header-only library to parse and emit **multi-format** configuration for your app. For example, you can parse JSON-file into the config and then compliment it from environment variables.

* [Quickstart](#quickstart)
* [Detailed description](#detailed-description)
    * [Configuration elements](#configuration-elements)
    * [Configuration formats](#configuration-formats)
        * [Environment](#environment)
        * [JSON](#json)
    * [Nested names](#nested-names)
    * [Optional elements](#optional-elements)
        * [uconfig::Variable](#uconfigvariable)
        * [uconfig::Vector](#uconfigvector)
    * [Multiformat configuration](#multiformat-configuration)
    * [Custom formats](#custom-formats)
    * [Custom types](#custom-types)
* [How to build](#how-to-build)

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

4. Call `Parse()` with proper format instance passed into it:
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

If any of mandatory variables is not parsed `Parse()` would throw an exception forbidding to use invalid `app_config`. You can override it by specifying `throw_on_fail = false` for:
```c++
template <typename F>
bool Parse(const F& parser, const std::string& path, const typename F::source_type* source, bool throw_on_fail = true);
```

5. Use variables as you normally do or dereference them via `*` or `->`:
```c++
if (app_config.timeout_ms > 0) {
    app.set_timeout(app_config.timeout_ms);
}

assert(app_config.nodes->size());
for (const auto& node : *app_config.nodes) {
    app.add_node(node.host, node.port);
}
```

**Note**:
JSON formatter uses [JSON-pointer](https://tools.ietf.org/html/rfc6901) as names identifier, therefore use `'/'` to delimit nested variables instead of `'_'` for env.

## Detailed description

### Configuration elements

All configuration elements (children) should be derived from `uconfig::Object` class, to achieve it just:
* Wrap plain variables, such as `int` or `double` into a `uconfig::Variable<T>`
* Declare sequence containers as `uconfig::Vector<T>`
* Use other configs derived from `uconfig::Config`

By default all elements are mandatory, meaning that `Parse()` or `Emit()` would throw an exception if element does not have a value. To declare element as optional see [Optional elements](#optional-elements). To continue parsing/emitting in case of not set mandatory values, pass `throw_on_fail = false` to:
```c++
template <typename F>
bool uconfig::Config<>::Parse(const F& parser, const std::string& path, const typename F::source_type* source, bool throw_on_fail);

template <typename F>
void uconfig::Config<>::Emit(const F& emitter, const std::string& path, typename F::dest_type* destination, bool throw_on_fail);
```

To check if element has a value call its' `Initialized()`, to check if it is optional – `Optional()`, to access a value just dereference or explicitly convert.

**Note**:
To see full definition of `uconfig::Object` and its' derivatives it look into sources.

### Configuration formats

There are several formats supported by the library.

#### Environment

Implemented as `uconfig::EnvFormat`.

Obtain and parse values from environment via [getenv()](https://man7.org/linux/man-pages/man3/getenv.3.html), emits to `std::map<std::string, std::string>`.

Since environment is just plain key-value string storage, this format implemented as converter from string and to string for basic types. Names of configuration elements are just names of env-variables. Elements of `uconfig::Vector` will have trailing `"_N"` to the name.

Supports:
* `short int`
* `unsigned short int`
* `int`
* `unsigned int`
* `long int`
* `unsigned long int`
* `long long int`
* `unsigned long long int`
* `std::string`

**Note**:
Does not support `bool` by default. To enable provide specialization.

#### JSON

Implemented as `uconfig::RapidjsonFormat`

Parse values from JSON-objects via [Rapidjson](https://rapidjson.org/), emits to `rapidjson::Document`.

JSON is typed format with hierarchical structure and this format implemented as get/set with type safety checks. Names of configuration elements are treated as JSON-pointers. Elements of `uconfig::Vector` will have trailing `"/N"` to the name.

Supports:
* `bool`
* `int`
* `unsigned int`
* `long int`
* `unsigned long int`
* `long long int`
* `unsigned long long int`
* `std::string`

### Nested names

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
struct GlobalConfig: public uconfig::Config<uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> log_file;
    uconfig::Variable<unsigned> log_rotate_mb;
    uconfig::Variable<std::string> upsteam_url;
    uconfig::Variable<unsigned> upsteam_timeout_ms{100};
    uconfig::Variable<std::string> server_host;
    uconfig::Variable<unsigned> server_port;

    using uconfig::Config<uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string&) override
    {
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
struct LogConfig: public uconfig::Config<uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> file;
    uconfig::Variable<unsigned> rotate_mb;

    using uconfig::Config<uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::RapidjsonFormat<>>(config_path + "/file", &file);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/rotate_mb", &rotate_mb);
    }
};

struct UpstreamConfig: public uconfig::Config<uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> url;
    uconfig::Variable<unsigned> timeout_ms{100};

    using uconfig::Config<uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::RapidjsonFormat<>>(config_path + "/url", &upsteam_url);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/timeout_ms", &upsteam_timeout_ms);
    }
};

struct ServerConfig: public uconfig::Config<uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> server_host;
    uconfig::Variable<unsigned> server_port;

    using uconfig::Config<uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::RapidjsonFormat<>>(config_path + "/host", &server_host);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/port", &server_port);
    }
};

struct GlobalConfig: public uconfig::Config<uconfig::RapidjsonFormat<>>
{
    LogConfig log_config;
    UpstreamConfig upstream_config;
    ServerConfig server_config;

    using uconfig::Config<uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string&) override
    {
        Register<uconfig::RapidjsonFormat<>>("/log", &log_config);
        Register<uconfig::RapidjsonFormat<>>("/upsteam", &upstream_config);
        Register<uconfig::RapidjsonFormat<>>("/server", &server_config);
    }
};
```

### Optional elements

All configuration elements can be defined as optional, for different types it is done differently and has different meaning.

#### `uconfig::Variable`

Variable considered optional if it constructed with some value:
```c++
uconfig::Variable<unsigned> port{8080};
```
defines `port` variable to be optional with default value = `8080`.

* If optional variable is not parsed from the source, it has default value.
* Parser won't stop if failed to lookup optional variable in the source.
* Emitter would emit optional variable with it's value whether it default or not.

#### `uconfig::Vector`

Vector considered optional if is constructed with default `std::vector` value or with `true`:
```c++
uconfig::Vector<int> all_numbers{true};
uconfig::Vector<int> selected_numbers{{1, 2, 3, 4, 5}};
uconfig::Vector<int> blacklisted_numbers{{}};
```
defines optional vector `all_numbers` **without** default value, `selected_numbers` – optional with `std::vector<int>{1, 2, 3, 4, 5}` as default value, `blacklisted_numbers` – optional empty by-default vector.

* If optional vector is not parsed from the source, it has default value if any.
* Parser won't stop if failed to lookup optional vector in the source.
* Emitter would emit **only non-empty** optional vectors.

### Multiformat configuration

If you application requires a configuration in multiple formats you should specify all of them as template parameters for `uconfig::Config` and register all elements within `Init()` for all formats:

```c++
struct ServerConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<std::string> server_host;
    uconfig::Variable<unsigned> server_port;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        // register with names
        Register<uconfig::EnvFormat>(config_path + "_HOST", &server_host);
        Register<uconfig::EnvFormat>(config_path + "_PORT", &server_port);
        // register with json-pointers
        Register<uconfig::RapidjsonFormat<>>(config_path + "/host", &server_host);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/port", &server_port);
    }
};
```

Upon parsing the config you can use `throw_on_fail` parameter to ignore absent mandatory for specific format:
```c++
// Only port set in JSON
rapidjson::Document config_json{rapidjson::kObjectType};
config_json.AddMember("port", 8080, config_json.GetAllocator());
// Only host set in env
setenv("SRV_HOST", "0.0.0.0", 1);

ServerConfig srv_config;
uconfig::RapidjsonFormat<> json_fmt;
srv_config.Parse(json_fmt, "", config_json, false); // this will ignore absent /host
// At this point srv_config still invalid because /host is not set
assert(!srv_config.Initialized());

uconfig::EnvFormat env_fmt;
srv_config.Parse(env_fmt, "SRV", nullptr); // this will validate host from env and port from JSON
// srv_config should be valid otherwise Parse() would throw
assert(srv_config.Initialized());
```

It allows to specify most of the configuration parameters via bulky JSON-file and overwrite or set some of them via env-variables, for example, production/qa specific.

### Custom formats
### Custom types


## How to build

This library is header-only, building required only for unit-tests. Prefer [out-of-source](https://gitlab.kitware.com/cmake/community/-/wikis/FAQ#what-is-an-out-of-source-build) building:

To install:
```bash
mkdir build
cd build
cmake ..
make install
```

To test:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON ..
make -j$(nproc)
ctest -S
```

### Cmake options

* **CMAKE_BUILD_TYPE** - [build type](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html). `RelWithDebInfo` by default.
* **BUILD_TESTING** - [build tests or not](https://cmake.org/cmake/help/latest/module/CTest.html). `OFF` by default.
* **UCONFIG_BUNDLED_MODE** - if the library is being built as a part of another project. Defined by `"${PROJECT_SOURCE_DIR}" == "${CMAKE_SOURCE_DIR}"` by default.

## License

Developed at **Tinkoff.ru** in 2021.\
Distibuted under **Apache License 2.0** [LICENSE](./LICENSE). You may also obtain this license at https://www.apache.org/licenses/LICENSE-2.0.

## Contacts

Author - i.s.vovk@tinkoff.ru\
Current maintainer - i.s.vovk@tinkoff.ru
