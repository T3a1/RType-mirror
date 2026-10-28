from conan import ConanFile
from conan.tools.cmake import CMakeToolchain


class RType(ConanFile):
    """Dependencies of the R-Type server, client and tests.

    The option `graphics` picks the client's graphics library (raylib or sfml):
    `conan install . -o "&:graphics=sfml" ...`. The CMake toolchain passes it
    on as RTYPE_GRAPHICS_BACKEND.
    """

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    options = {"graphics": ["raylib", "sfml"]}
    default_options = {
        "graphics": "raylib",
        # The client only uses SFML's window and graphics modules.
        "sfml/*:audio": False,
        "sfml/*:network": False,
    }

    def requirements(self):
        self.requires("asio/1.36.0")
        if self.options.graphics == "raylib":
            self.requires("raylib/5.5")
        else:
            self.requires("sfml/3.0.2")

    def build_requirements(self):
        self.test_requires("gtest/1.15.0")

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.variables["RTYPE_GRAPHICS_BACKEND"] = str(self.options.graphics)
        toolchain.generate()
