from conans import ConanFile, CMake, tools


class Pkg(ConanFile):
    name = "uconfig"
    version = "2.0.1"
    license = "Apache-2.0"
    author = "Ivan Vovk (i.s.vovk@tinkof.ru)"
    url = "https://github.com/TinkoffCreditSystems/uconfig"
    description = "C++ library to parse and emit multi-format configuration for your app"
    topics = ("configuration")

    options = {"testing": [True, False]}
    default_options = {"testing": False}

    generators = "cmake_find_package"
    exports_sources = "include/*", "CMakeLists.txt", "uconfigConfig.cmake.in"

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTING"] = "ON" if self.options.testing else "OFF"
        cmake.configure()

        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.info.header_only()
