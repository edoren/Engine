#!/usr/bin/env python3

import argparse
import os
import os.path
import platform

from scripts import file_utils

platform_choices = []
if platform.system() == "Windows":
    platform_choices = ["windows", "android"]
if platform.system() == "Linux":
    platform_choices = ["linux", "android"]
if platform.system() == "Darwin":
    platform_choices = ["macosx", "android"]

parser = argparse.ArgumentParser(
    description="Generate Engine project files for the selected platform.")
parser.add_argument("--name", required=True, type=str,
                    help="The application name")
parser.add_argument("--package", required=True, type=str,
                    help="The application package id")
parser.add_argument("--platform", required=True, choices=platform_choices,
                    help="The name of the platform to build")
parser.add_argument("--build-type", choices=["Release", "Debug"],
                    default="Debug",
                    help="The build type, Release or Debug")
parser.add_argument("--cmake-args", nargs="*", metavar="CMAKE_ARG=VALUE",
                    default=[], help="CMake additional arguments")

args = parser.parse_args()

# Configure CMake
cmake_arguments = dict([arg.split("=") for arg in args.cmake_args])
if "CMAKE_BUILD_TYPE" not in cmake_arguments:
    cmake_arguments["CMAKE_BUILD_TYPE"] = args.build_type
else:
    args.build_type = cmake_arguments["CMAKE_BUILD_TYPE"]

# Dictionary used to configure the project files
config = {}
config["APP_NAME"] = args.name
config["APP_BUILD_TYPE"] = args.build_type
config["APP_PACKAGE_ID"] = args.package
config["APP_ROOT_DIRECTORY"] = os.path.dirname(os.path.realpath(__file__))
config["APP_BUILD_DIRECTORY"] = os.path.join(config["APP_ROOT_DIRECTORY"],
                                             "build", args.platform)
config["CMAKE_ARGUMENTS"] = " ".join(["-D" + arg[0] + "=" + arg[1]
                                      for arg in cmake_arguments.items()])

if __name__ == "__main__":
    if (not file_utils.create_directory(config["APP_BUILD_DIRECTORY"])):
        print("Could not create directory: " + config["APP_BUILD_DIRECTORY"])

    thidparty_dir = os.path.join(config["APP_ROOT_DIRECTORY"], "third_party")

    print("Build directory: " + config["APP_BUILD_DIRECTORY"])

    if args.platform == "android":
        android_sdk_home = os.environ.get("ANDROID_HOME")
        android_ndk_home = os.environ.get("ANDROID_NDK_HOME")

        android_project = os.path.join(config["APP_ROOT_DIRECTORY"],
                                       "projects", "android")
        java_src_dir = os.path.join(config["APP_BUILD_DIRECTORY"], "src")
        assets_dir = os.path.join(config["APP_BUILD_DIRECTORY"], "assets")
        jniLibs_dir = os.path.join(config["APP_BUILD_DIRECTORY"], "jniLibs")

        # Copy and configure the android project
        print("Configuring build folder")
        file_utils.configure_directory(android_project,
                                       config["APP_BUILD_DIRECTORY"], config)

        # Fix the App Java file package
        print("Moving Java files to the package: " + args.package)
        java_src_file = os.path.join(java_src_dir, "MainActivity.java")
        package_dir = os.path.join(java_src_dir,
                                   os.sep.join(args.package.split(".")))
        file_utils.create_directory(package_dir)
        file_utils.move_file(java_src_file, package_dir, force=True)

        # Create additional folders
        file_utils.create_directory(assets_dir)
        file_utils.create_directory(jniLibs_dir)

        # Copy the thidparty Java files
        print("Copying thidparty Java files")
        thidparty_java_dirs = [
            os.path.join(thidparty_dir, "sdl2", "android-project", "src")
        ]
        for directory in thidparty_java_dirs:
            file_utils.copy_directory(directory, java_src_dir, force=True)

        # Copy the validation layers to the folder
        if args.build_type == "Debug" and android_ndk_home is not None:
            print("Copying Vulkan validation layers")
            layer_dir = os.path.join(android_ndk_home, "sources",
                                     "third_party", "vulkan", "src",
                                     "build-android", "jniLibs")
            file_utils.copy_directory(layer_dir, jniLibs_dir, force=True)

# TODO
# 1. Create a build script after generate the project
# 2. Add build for Windows, Linux and MacOSX
