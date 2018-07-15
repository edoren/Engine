#!/usr/bin/env python3

import errno
import os
import os.path
import platform
import glob
import shutil
import sys
from subprocess import run

from file_utils import FileUtils


def find_glslc_executable():
    shader_compiler = "glslc"
    search_paths = []

    if platform.system() == "Windows":
        shader_compiler += ".exe"

    # Try to find glslc from the path
    glslc_exe = FileUtils.which(shader_compiler)
    if glslc_exe is not None:
        return glslc_exe

    # Try to find the shader compiler in the Vulkan SDK
    if platform.system() == "Windows":
        matches = glob.glob("C:\\VulkanSDK\\*\\")
        for match in matches:
            if os.path.isdir(match):
                search_paths.append(os.path.join(match, "Bin"))
    vulkan_sdk_dir = os.environ.get("VULKAN_SDK")
    if vulkan_sdk_dir is not None:
        vulkan_sdk_dir = os.path.abspath(vulkan_sdk_dir)
        search_paths.append(os.path.join(vulkan_sdk_dir, "bin"))

    # Try to find the shader compiler using the Android NDK
    android_ndk_dir = os.environ.get("ANDROID_NDK_HOME")
    if android_ndk_dir is not None:
        android_ndk_dir = os.path.abspath(android_ndk_dir)
        glob_pattern = os.path.join(android_ndk_dir, "shader-tools", "*")
        glob_list = FileUtils.glob(glob_pattern)
        search_paths += glob_list

    # Try to find it in the PATH
    path_env = os.environ["PATH"]
    if "PATH" in os.environ:
        search_paths += path_env.split(os.pathsep)

    # Search the shader compiler
    shader_compiler_exe = None
    for path in search_paths:
        if os.path.exists(path):
            file_path = os.path.join(path, shader_compiler)
            if os.path.isfile(file_path):
                shader_compiler_exe = file_path
                break

    if shader_compiler_exe is None:
        print(shader_compiler + " not found. Is the Vulkan SDK installed?")

    return shader_compiler_exe


def generate_spirv_shaders(data_folder):
    glslc_exe = find_glslc_executable()
    if glslc_exe is None:
        print("Error: glslc not found")
        exit(1)

    glsl_shaders_folder = os.path.join(data_folder, "shaders", "glsl")
    spirv_shaders_folder = os.path.join(data_folder, "shaders", "spirv")

    glsl_shaders = [f for f in os.listdir(glsl_shaders_folder)
                    if os.path.isfile(os.path.join(glsl_shaders_folder, f))]

    processed = 0
    for shader_basename in glsl_shaders:
        glsl_shader_path = os.path.join(glsl_shaders_folder, shader_basename)
        spriv_shader_path = os.path.join(spirv_shaders_folder, shader_basename)

        processed += 1
        percentage = int(100 * (processed / float(len(glsl_shaders))))
        print_format = "[{:>3}%] Compiling shader {}"
        print(print_format.format(percentage, spriv_shader_path),
              flush=True)

        std = "450core"
        # std = "320es"

        glslc_command = [
            glslc_exe,
            "-std={}".format(std),
            glsl_shader_path,
            "-o",
            spriv_shader_path
        ]
        run(glslc_command, stdout=open(os.devnull, 'wb'), check=True)


def main(argv):
    if len(argv) != 3:
        print("Usage:\n"
              "{} input_folder output_folder".format(argv[0]))
        exit()

    input_folder = os.path.abspath(argv[1])
    output_folder = os.path.abspath(argv[2])
    if not os.path.exists(input_folder):
        print("Target folder does not exist", 1)
        exit(1)

    print("================== Processing engine data ==================")
    FileUtils.cp_r(input_folder, output_folder, force=True)
    generate_spirv_shaders(output_folder)
    print("============================================================")


if __name__ == "__main__":
    main(sys.argv)
