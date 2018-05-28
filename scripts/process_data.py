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


def generate_spirv_shaders(data_folder):
    shader_compiler = "glslangValidator"

    search_paths = []
    if platform.system() == "Windows":
        shader_compiler += ".exe"
        matches = glob.glob("C:\\VulkanSDK\\*\\")
        for match in matches:
            if os.path.isdir(match):
                search_paths.append(os.path.join(match, "Bin"))
    if "VULKAN_SDK" in os.environ:
        search_paths.append(os.path.join(os.environ["VULKAN_SDK"], "bin"))
    if "PATH" in os.environ:
        search_paths += os.environ["PATH"].split(os.pathsep)

    shader_compiler_exe = None
    for path in search_paths:
        if os.path.exists(path):
            file_path = os.path.join(path, shader_compiler)
            if os.path.isfile(file_path):
                shader_compiler_exe = file_path
                break

    if shader_compiler_exe is None:
        print("Vulkan SDK not installed")
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
        print("[{:>3}%] Compiling shader ".format(percentage),
              end="", flush=True)

        run([shader_compiler_exe, "-V", glsl_shader_path, "-o", spriv_shader_path])


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
