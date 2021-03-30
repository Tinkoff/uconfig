# uconfig

[![Language C++](https://img.shields.io/badge/language-c++-blue.svg)](https://isocpp.org)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

C++ header-only library to parse and emit **multi-format** configuration for your app. For example, you can parse JSON-file into the config and then compliment it from environment variables.

It requires c++17 compatible compiler and [Rapidjson](https://rapidjson.org/) if you're planning to use JSON formatting.

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
    * [Value validation](#value-validation)
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
    virtual void Init(const std::string& env_prefix) override
    {
        Register<uconfig::EnvFormat>(env_prefix + "_FILE", &file);
        Register<uconfig::EnvFormat>(env_prefix + "_ROTATE_MB", &rotate_mb);
    }
};

struct NodeConfig: public uconfig::Config<uconfig::EnvFormat>
{
    uconfig::Variable<std::string> host; // mandatory string
    uconfig::Variable<unsigned> port; // mandatory unsigned

    using uconfig::Config<uconfig::EnvFormat>::Config;

    virtual void Init(const std::string& env_prefix) override
    {
        Register<uconfig::EnvFormat>(env_prefix + "_HOST", &host);
        Register<uconfig::EnvFormat>(env_prefix + "_PORT", &port);
    }
};

struct AppConfig: public uconfig::Config<uconfig::EnvFormat>
{
    LogConfig log_config; // mandatory LogConfig
    uconfig::Variable<unsigned> timeout_ms{100}; // optional unsigned with default = 100
    uconfig::Vector<NodeConfig> nodes; // mandatory vector of NodeConfig
    uconfig::Vector<std::string> endpoints; // mandatory vector of strings

    using uconfig::Config<uconfig::EnvFormat>::Config;

    virtual void Init(const std::string& env_prefix) override
    {
        Register<uconfig::EnvFormat>(env_prefix + "_LOG", &log_config);
        Register<uconfig::EnvFormat>(env_prefix + "_TIMEOUT_MS", &timeout_ms);
        Register<uconfig::EnvFormat>(env_prefix + "_NODE", &nodes);
        Register<uconfig::EnvFormat>(env_prefix + "_ENDPOINT", &endpoints);
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

If any of mandatory variables is not parsed `Parse()` would throw an exception forbidding to use invalid `app_config`. You can override it by specifying `throw_on_fail = false` for `Parse()`.

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
* `float`
* `double`
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
* `float`
* `double`
* `std::string`

### Nested names

Full name for the variable formed by nested calls of `void Config<>::Init(const std::string& config_path)` with parent name passed as `config_path`.
By design variables naming scheme is arbitrary, meaning you are free to call `Register()` with whatever string you want. But if format supports hierarchy, names should respect it and represent some hierarchical paths.

For example, JSON-objects have hierarchy and `uconfig::RapidjsonFormat` uses JSON-pointer for variable names, so
```c++
Register<uconfig::RapidjsonFormat<>("/a/b/c", &variable);
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
        Register<uconfig::RapidjsonFormat<>>(config_path + "/url", &url);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/timeout_ms", &timeout_ms);
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

All configuration elements can be defined as optional. For different types it is done differently and has different meaning.

#### `uconfig::Variable`

Variable considered optional if it is constructed with some value:
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
defines optional vector `all_numbers` without default value, `selected_numbers` – optional with `std::vector<int>{1, 2, 3, 4, 5}` as default value, `blacklisted_numbers` – optional empty by-default vector.

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

Upon parsing the config you can use `throw_on_fail` parameter to ignore absent mandatory elements in specific format:
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

To implement custom format you need to derive from `uconfig::Format` and define/override all of its' functions. Its' interface is self-explanatory, so:
```c++
class Format
{
public:
    /// Name of the format. Used to form nice error-strings.
    static inline const std::string name = "[NO FORMAT]";
    /// Source of the format to parse from.
    using source_type = void;
    /// Destination of the format to emit to.
    using dest_type = void;

    /**
     * Parse the value at @p path from @p source.
     *
     * @tparam T Type to parse.
     *
     * @param[in] source Source to parse value from.
     * @param[in] path Path where the value resides in @p source.
     *
     * @returns Value wrapped in std::optional or std::nullopt.
     */
    template <typename T>
    std::optional<T> Parse(const source_type* source, const std::string& path) const;

    /**
     * Emit the value at @p path to @p dest.
     *
     * @tparam T Type to emit.
     *
     * @param[in] dest Destination to emit to.
     * @param[in] path Path where to emit.
     * @param[in] value Value to emit.
     */
    template <typename T>
    void Emit(dest_type* dest, const std::string& path, const T& value) const;

    /**
     * Construct path to a vector element at @p index accroding to the format.
     *
     * @param[in] vector_path Path to the vector itself.
     * @param[in] index Position in the vector to make path to.
     *
     * @returns Path to the vector element at @p index.
     */
    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) const noexcept = 0;
};
```

`Parse<T>()` and `Emit<T>()` will be called for all types used for `uconfig::Variable<T>` and `uconfig::Vector<T>` in your configs. For examples you can look into `uconfig::EnvFormat` or `uconfig::RapidjsonFormat` implementation.

### Custom types

If you want config parameters to be a `enum` or some other custom type you need to provide specializations for functions:
```c++
template <typename T>
std::optional<T> Parse(const source_type* source, const std::string& path) const;

template <typename T>
void Emit(dest_type* dest, const std::string& path, const T& value) const;
```
of desired format. For example:

```c++
enum class LogLevel
{
    FATAL,
    CRITICAL,
    WARNING,
    INFO,
    DEBUG,
    VERBOSE,
};

template <>
std::optional<LogLevel> uconfig::EnvFormat::Parse<Log::Level>(const void*, const std::string& path)
{
    const char* env_var = std::getenv(path.c_str());
    if (!env_var) {
        return std::nullopt;
    }

    if (lvl_str == "fatal") {
        return LogLevel::FATAL;
    } else if (lvl_str == "critical") {
        return LogLevel::CRITICAL;
    } else if (lvl_str == "warning") {
        return LogLevel::WARNING;
    } else if (lvl_str == "info") {
        return LogLevel::INFO;
    } else if (lvl_str == "debug") {
        return LogLevel::DEBUG;
    } else if (lvl_str == "verbose") {
        return LogLevel::VERBOSE;
    }

    return std::nullopt;
}

template <>
void uconfig::EnvFormat::Emit<LogLevel>(const LogLevel& value, const std::string& path, std::map<std::string, std::string>* dest)
{
    switch (lvl) {
    case LogLevel::FATAL:
        dest->emplace(path, "FATAL");
    case LogLevel::CRITICAL:
        dest->emplace(path, "ERROR");
    case LogLevel::WARNING:
        dest->emplace(path, "WARN");
    case LogLevel::INFO:
        dest->emplace(path, "INFO");
    case LogLevel::DEBUG:
        dest->emplace(path, "DEBUG");
    case LogLevel::VERBOSE:
        dest->emplace(path, "TRACE");
    }
}
```

Then you can use it in the config:
```c++
struct LogConfig: public uconfig::Config<uconfig::EnvFormat>
{
    uconfig::Variable<LogLevel> level;

    using uconfig::Config<uconfig::EnvFormat>::Config;

    virtual void Init(const std::string& env_prefix) override
    {
        Register<uconfig::EnvFormat>(env_prefix + "_LEVEL", &level);
    }
};
```

### Value validation

If you want to validate values you should override `virtual void Validate() const` for new element. This function get called after value has been parsed and will cause `Parse()` to throw an exception if `throw_on_fail = true`.

For example:

```c++
struct EvenInteger: public uconfig::Variable<int>
{
    using uconfig::Variable<int>::Variable;

    virtual void Validate() const override
    {
        if (Get() % 2) {
            throw std::runtime_error(std::to_string(Get()) + " is not even");
        }
    }
};
```

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
* **BUILD_TESTING** - build included unit-tests. `OFF` by default.

## License

Developed at **Tinkoff.ru** in 2021.\
Distibuted under **Apache License 2.0** [LICENSE](./LICENSE). You may also obtain this license at https://www.apache.org/licenses/LICENSE-2.0.

## Contacts

Author - i.s.vovk@tinkoff.ru\
Current maintainer - i.s.vovk@tinkoff.ru
