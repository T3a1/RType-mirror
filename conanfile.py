from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain


class RType(ConanFile):
    """Dependencies of the R-Type server, client and tests.

    The option `graphics` picks the client's graphics library (raylib, sfml or
    ncurses):
    `conan install . -o "&:graphics=sfml" ...`. The CMake toolchain passes it
    on as RTYPE_GRAPHICS_BACKEND.
    """

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    options = {"graphics": ["raylib", "sfml", "ncurses"]}
    default_options = {
        "graphics": "raylib",
        # The client only uses SFML's window and graphics modules.
        "sfml/*:audio": False,
        "sfml/*:network": False,
        # The ncurses client only needs the library itself.
        "ncurses/*:with_cxx": False,
        "ncurses/*:with_progs": False,
    }

    def validate(self):
        if self.options.graphics == "ncurses" and self.settings.os == "Windows":
            raise ConanInvalidConfiguration("the ncurses client only runs on Linux")

    def requirements(self):
        self.requires("asio/1.36.0")
        if self.options.graphics == "raylib":
            self.requires("raylib/5.5")
        elif self.options.graphics == "sfml":
            self.requires("sfml/3.0.2")
        else:
            self.requires("ncurses/6.5")

    def build_requirements(self):
        self.test_requires("gtest/1.15.0")

    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.variables["RTYPE_GRAPHICS_BACKEND"] = str(self.options.graphics)
        toolchain.generate()
