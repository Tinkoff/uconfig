## 2.0.3 (2021-05-01)

### Misc
* Use rapidjson include folder if found
* Append CMAKE_MODULE_PATH
* Fix gcc 10 compile errors and warnings

## 2.0.2 (2021-04-20)

### Misc
* Require cmake 3.0+
* Add virtual dctor for abstract classes
* Move test -> tests
* Update README

## 2.0.1 (2021-03-13)

### Bug Fixes
* Do not loop indefinitely while parsing vector with default-value elements

### Misc
* Update README

# 2.0.0 (2021-03-10)

### Features
* Section -> Config
* Support Vector of plain types
* Support optional and default values for Vector
* Config template is multiformat
* Cofig::Register is format templated
* Add doxygen-format docs
* Add tests
* Add README
* Add CHANGELOG

# 1.2.0 (2020-09-11)

### Features
* Allow parser to continue if not all mandatory variables are defined

## 1.1.1 (2021-08-06)
### Bug Fixes
* Fix ParseError thrown twice with the same message in Parse()

### Misc
* Update README

# 1.1.0 (2020-06-22)

### Features
* Implement PostParse() for Section/Vector

## 1.0.1 (2020-04-08)

### Bug Fixes
* Throw proper exceptions in Parse()/Emit()

# 1.0.0 (2020-02-03)

- initial release

