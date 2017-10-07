#!/usr/bin/env python3

# Required Python version 3.5

import argparse
import os
import os.path
import platform
import stat

from scripts import file_utils

platform_choices = []
if platform.system() == "Windows":
    platform_choices = ["windows", "android"]
if platform.system() == "Linux":
    platform_choices = ["linux", "android"]
if platform.system() == "Darwin":
    platform_choices = ["macosx", "android"]

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


class CMakeBuildGenerator:
    def __init__(self, args):
        self.app_name = args.name
        self.app_package = args.package
        self.app_platform = args.platform
        self.app_build_type = args.build_type
        self.app_root_dir = file_utils.join(
            os.path.dirname(os.path.realpath(__file__)))
        self.app_build_dir = file_utils.join(self.app_root_dir, "build",
                                             args.platform)

        # Dictionary used to configure the project files
        self.file_config = {
            "APP_NAME": self.app_name,
            "APP_PACKAGE": self.app_package,
            "APP_PLATFORM": self.app_platform,
            "APP_BUILD_TYPE": self.app_build_type,
            "APP_ROOT_DIRECTORY": self.app_root_dir,
            "APP_BUILD_DIRECTORY": self.app_build_dir
        }

        # Create the CMake commandline arguments
        cmake_args_dict = self.file_config.copy()
        cmake_args_dict.update([arg.split("=") for arg in args.cmake_args])

        if "CMAKE_GENERATOR_PLATFORM" not in cmake_args_dict and \
                self.app_platform == "windows" and \
                platform.machine().endswith("64"):
            cmake_args_dict["CMAKE_GENERATOR_PLATFORM"] = "x64"

        self.cmake_args = [
            "-D{}='{}'".format(*arg) for arg in cmake_args_dict.items()
        ]

        self.file_config["CMAKE_ARGUMENTS"] = " ".join(self.cmake_args)

    def configure_android(self):
        android_sdk_home = os.environ.get("ANDROID_HOME")
        android_ndk_home = os.environ.get("ANDROID_NDK_HOME")

        android_project = file_utils.join(self.app_root_dir, "projects",
                                          "android")

        thirdparty_dir = file_utils.join(self.app_root_dir, "third_party")

        java_src_dir = file_utils.join(self.app_build_dir, "src")
        assets_dir = file_utils.join(self.app_build_dir, "assets")
        jniLibs_dir = file_utils.join(self.app_build_dir, "jniLibs")

        # Copy and configure the android project
        print("Configuring build directory")
        file_utils.configure_directory(android_project, self.app_build_dir,
                                       self.file_config)

        # Create additional folders
        file_utils.create_directory(assets_dir)
        file_utils.create_directory(jniLibs_dir)

        # Fix the App Java file package
        print("Moving Java files to the package: " + self.app_package)
        java_src_file = file_utils.join(java_src_dir, "MainActivity.java")
        package_dir = file_utils.join(java_src_dir,
                                      os.sep.join(self.app_package.split(".")))
        file_utils.create_directory(package_dir)
        file_utils.move_file(java_src_file, package_dir, force=True)

        # Copy the thirdparty Java files
        print("Copying thirdparty Java files")
        thirdparty_java_dirs = [
            file_utils.join(thirdparty_dir, "sdl2", "android-project", "src")
        ]
        for directory in thirdparty_java_dirs:
            file_utils.copy_directory(directory, java_src_dir, force=True)

        # Copy the validation layers to the folder
        if self.app_build_type == "Debug":
            if android_ndk_home is not None:
                print("Copying Vulkan validation layers")
                layer_dir = file_utils.join(android_ndk_home, "sources",
                                            "third_party", "vulkan", "src",
                                            "build-android", "jniLibs")
                file_utils.copy_directory(layer_dir, jniLibs_dir, force=True)
            else:
                print("Error copying Vulkan validation layers. Make sure you"
                      "have set the ANDROID_NDK_HOME environment variable and "
                      "that your have NDK r12 or later.")

    def configure_desktop(self):
        pass

    def configure_platform(self):
        print("==================================================")
        print("Parameters:")
        for name, value in self.file_config.items():
            print("    - {}: {}".format(name, value))
        print()
        print("==================================================")

        if os.path.isdir(self.app_build_dir):
            print("Cleaning up build directory")
            file_utils.remove_directory_contents(self.app_build_dir)
        else:
            print("Creating build directory")
            if not file_utils.create_directory(self.app_build_dir):
                print("Could not create build directory")
                return

        if self.app_platform in ["windows", "linux", "macosx"]:
            self.configure_desktop()
        elif self.app_platform == "android":
            self.configure_android()

    def create_build_script(self):
        print("Creating build script")
        build_file_path = file_utils.join(self.app_build_dir, "build.py")

        if os.path.isfile(build_file_path):
            os.remove(build_file_path)

        if self.app_platform in ["windows", "linux", "macosx"]:
            cmake_path = file_utils.find_executable("cmake")
            build_commands = [
                [cmake_path, self.app_root_dir] + self.cmake_args,
                [cmake_path, "--build", self.app_build_dir]
            ]
        elif self.app_platform == "android":
            if platform.system() == "Windows":
                gradle_executable = "gradlew.bat"
            else:
                gradle_executable = "gradlew"
            gradle_path = file_utils.join(self.app_build_dir,
                                          gradle_executable)
            build_commands = [[
                gradle_path, "assemble{}".format(self.app_build_type)
            ]]

        build_file_template = [
            "#!/usr/bin/env python3",
            "# File generated using the generate.py script. Do not modify it.",
            "from subprocess import run"
        ]
        for command in build_commands:
            build_file_template.append("run({}, check=True)".format(command))

        with open(build_file_path, 'w') as build_file:
            for line in build_file_template:
                build_file.write(line + "\n")

        st = os.stat(build_file_path)
        os.chmod(build_file_path, st.st_mode | stat.S_IXUSR)


if __name__ == "__main__":
    generator = CMakeBuildGenerator(args)
    generator.configure_platform()
    generator.create_build_script()
    print("==================================================")