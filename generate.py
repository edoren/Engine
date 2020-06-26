#!/usr/bin/env python3

# Required Python version 3.5

import argparse
import os
import os.path
import platform
import re
import stat
import subprocess
import sys

from scripts.file_utils import FileUtils

platform_choices = []
if platform.system() == "Windows":
    platform_choices = ["windows", "android"]
if platform.system() == "Linux":
    platform_choices = ["linux", "android"]
if platform.system() == "Darwin":
    platform_choices = ["macos", "ios", "android"]

cmake_define_regex = re.compile('^-D([A-Za-z_-]+)=[\'"]?([^\'"]+)[\'"]?$')

parser = argparse.ArgumentParser(description="Generate Engine project files "
                                 "for the selected platform.")
parser.add_argument(
    "--name",
    required=True,
    type=str,
    dest="name",
    help="The application name")
parser.add_argument(
    "--package",
    required=True,
    type=str,
    dest="package",
    help="The application package id")
parser.add_argument(
    "--platform",
    required=True,
    choices=platform_choices,
    dest="platform",
    help="The name of the platform to build")
parser.add_argument(
    "--build-type",
    choices=["Release", "Debug"],
    dest="build_type",
    default="Debug",
    help="The build type, Release or Debug")
parser.add_argument(
    "--cmake-args",
    nargs="*",
    metavar="CMAKE_ARG=VALUE",
    dest="cmake_args",
    default=[],
    help="CMake additional arguments")

args = parser.parse_args()

cmake_path = FileUtils.which("cmake")

if cmake_path is None:
    print("Error: CMake not installed")
    exit()


class CMakeBuildGenerator:
    def __init__(self, args):
        self.app_name = args.name
        self.app_package = args.package
        self.app_platform = args.platform
        self.app_build_type = args.build_type
        self.app_root_dir = FileUtils.join(
            os.path.dirname(os.path.realpath(__file__)))
        self.app_build_dir = FileUtils.join(self.app_root_dir, "build",
                                            args.platform)

        # Dictionary used to configure the project files
        self.file_config = {
            "APP_NAME": self.app_name,
            "APP_PACKAGE": self.app_package,
            "APP_PLATFORM": self.app_platform,
            "APP_BUILD_TYPE": self.app_build_type,
            "APP_ROOT_DIRECTORY": self.app_root_dir,
            "APP_BUILD_DIRECTORY": self.app_build_dir,
            "PYTHON_EXECUTABLE": FileUtils.join(sys.executable)
        }

        # Create the CMake commandline arguments
        self.cmake_args = []
        cmake_args_dict = self.file_config.copy()
        cmake_args_dict.update([arg.split("=") for arg in args.cmake_args])
        self.add_cmake_args(cmake_args_dict)

    def configure_platform(self):
        print("==================================================")
        if os.path.isdir(self.app_build_dir):
            print("Cleaning up build directory")
            FileUtils.rm_rf(self.app_build_dir)

        print("Creating build directory")
        FileUtils.mkdir_p(self.app_build_dir)
        if not os.path.isdir(self.app_build_dir):
            print("Could not create build directory")
            return

        self.add_cmake_arg("--no-warn-unused-cli")

        if self.app_platform in ["windows", "linux", "macos"]:
            self.configure_desktop()
        elif self.app_platform == "android":
            self.configure_android()
        elif self.app_platform == "ios":
            self.configure_ios()

        print("==================================================")
        print("CMake Parameters:")
        for value in self.cmake_args:
            match = cmake_define_regex.match(value)
            if match is not None:
                print("    - {}: {}".format(match.group(1), match.group(2)))
        print()
        print("==================================================")

    def configure_android(self):
        android_sdk_home = os.environ.get("ANDROID_HOME")
        android_ndk_home = os.environ.get("ANDROID_NDK_HOME")

        android_project = FileUtils.join(self.app_root_dir, "projects",
                                         "android")

        thirdparty_dir = FileUtils.join(self.app_root_dir, "third_party")

        java_src_dir = FileUtils.join(self.app_build_dir, "src")
        assets_dir = FileUtils.join(self.app_build_dir, "assets")
        jniLibs_dir = FileUtils.join(self.app_build_dir, "jniLibs")

        # Copy and configure the android project
        print("Configuring build directory")
        cmake_args_str = ", ".join(['"{}"'.format(arg)
                                    for arg in self.cmake_args])
        self.file_config["CMAKE_ARGUMENTS"] = cmake_args_str
        FileUtils.configure_directory(android_project, self.app_build_dir,
                                      self.file_config)

        # Create additional folders
        FileUtils.mkdir_p(assets_dir)
        FileUtils.mkdir_p(jniLibs_dir)

        # Fix the App Java file package
        print("Moving Java files to the package: " + self.app_package)
        java_src_file = FileUtils.join(java_src_dir, "MainActivity.java")
        package_dir = FileUtils.join(java_src_dir,
                                     os.sep.join(self.app_package.split(".")))
        FileUtils.mkdir_p(package_dir)
        FileUtils.mv(java_src_file, package_dir, force=True)

        # Copy the validation layers to the folder
        if self.app_build_type == "Debug":
            if android_ndk_home is not None:
                print("Copying Vulkan validation layers")
                layer_dir = FileUtils.join(android_ndk_home, "sources",
                                           "third_party", "vulkan", "src",
                                           "build-android", "jniLibs")
                FileUtils.cp_r(layer_dir, jniLibs_dir, force=True)
            else:
                print("Error copying Vulkan validation layers. Make sure you"
                      "have set the ANDROID_NDK_HOME environment variable and "
                      "that your have NDK r12 or later.")

        # Copy the thirdparty .so Release libraries
        prebuilt_dir = FileUtils.join(thirdparty_dir, "prebuilt", "android")
        prebuilt_archs_dir = [o for o in os.listdir(prebuilt_dir)
                              if os.path.isdir(FileUtils.join(prebuilt_dir, o))]
        for arch_dir in prebuilt_archs_dir:
            jni_arch_dir = FileUtils.join(jniLibs_dir, arch_dir)
            prebuilt_arch_dir = FileUtils.join(
                prebuilt_dir, arch_dir, "Release")
            soLibs = FileUtils.glob(FileUtils.join(prebuilt_arch_dir, "*.so"))
            for soLib in soLibs:
                FileUtils.cp(soLib, jni_arch_dir)

    def get_cmake_argument(self, argument):
        for value in self.cmake_args:
            match = cmake_define_regex.match(value)
            if match is not None and match.group(1) == argument:
                return match.group(2)
        return None

    def configure_ios(self):
        toolchain_file = os.path.join(self.app_root_dir, "cmake", "Toolchains",
                                      "ios.toolchain.cmake")
        cmake_generator = [
            ("CMAKE_GENERATOR", "Xcode"),
            ("CMAKE_TOOLCHAIN_FILE", toolchain_file),
            ("PLATFORM", "OS64COMBINED"),
            ("DEPLOYMENT_TARGET", "13.0"),
        ]
        self.add_cmake_args(cmake_generator)

    def configure_desktop(self):
        gen = self.get_cmake_argument("CMAKE_GENERATOR")
        gen_plat = self.get_cmake_argument("CMAKE_GENERATOR_PLATFORM")
        if self.app_platform == "macos" and gen is None:
            self.add_cmake_arg("CMAKE_GENERATOR", "Unix Makefiles")
        if self.app_platform == "windows":
            if gen_plat is None and platform.machine().endswith("64"):
                self.add_cmake_arg("CMAKE_GENERATOR_PLATFORM", "x64")
            if gen is None:
                self.add_cmake_arg("CMAKE_GENERATOR", "Visual Studio 16 2019")

    def create_build_script(self):
        build_file_path = FileUtils.join(self.app_build_dir, "build.py")

        if os.path.isfile(build_file_path):
            os.remove(build_file_path)

        if self.app_platform in ["windows", "linux", "macos", "ios"]:
            command = [cmake_path, self.app_root_dir] + self.cmake_args
            subprocess.run(command, cwd=self.app_build_dir, check=True)
            build_commands = [
                [cmake_path, "--build", self.app_build_dir,
                 "--config", self.app_build_type]
            ]
            print("==================================================")
        elif self.app_platform == "android":
            if platform.system() == "Windows":
                gradle_executable = "gradlew.bat"
            else:
                gradle_executable = "gradlew"
            gradle_path = FileUtils.join(self.app_build_dir,
                                         gradle_executable)
            build_commands = [
                [gradle_path, "assemble{}".format(self.app_build_type)]
            ]

        build_file_template = [
            "#!/usr/bin/env python3",
            "# File generated using the generate.py script. Do not modify it.",
            "from subprocess import run"
        ]
        for command in build_commands:
            build_file_template.append("run({}, cwd='{}', check=True)"
                                       .format(command, self.app_build_dir))

        with open(build_file_path, 'w') as build_file:
            for line in build_file_template:
                build_file.write(line + "\n")

        FileUtils.chmod("u+x", build_file_path)
        print("Build script created in folder:", self.app_build_dir)

    def add_cmake_args(self, args):
        if type(args) == dict:
            self.add_cmake_args(list(args.items()))
        if type(args) == list:
            for pair in reversed(args):
                self.add_cmake_arg(pair[0], pair[1])

    def add_cmake_arg(self, arg_name, value=None):
        if value is None:
            self.cmake_args.append(arg_name)
        else:
            self.cmake_args.insert(0, "-D{}='{}'".format(arg_name, value))


if __name__ == "__main__":
    generator = CMakeBuildGenerator(args)
    generator.configure_platform()
    generator.create_build_script()
    print("==================================================")
