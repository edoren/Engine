#!/usr/bin/env python3

import os
import os.path
import sys
import errno
import shutil
import subprocess


def copy_directory(src, dst):
    try:
        shutil.copytree(src, dst)
    except OSError as exc:  # python >2.5
        if exc.errno == errno.ENOTDIR:
            shutil.copy(src, dst)
        else:
            raise


def remove_directory(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)


def get_vulkan_sdk_folder():
    vulkan_dir = os.environ.get("VULKAN_SDK")
    if vulkan_dir is None:
        exit_with_msg("Vulkan SDK not installed", 1)
    return os.path.abspath(vulkan_dir)


def exit_with_msg(message, code=0):
    print(message)
    exit(0)


def generate_spirv_shaders(data_folder):
    vulkan_dir = get_vulkan_sdk_folder()

    glslang_exe = os.path.join(vulkan_dir, "bin", "glslangValidator")

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

        command = "{} -V {} -o {}".format(glslang_exe,
                                          glsl_shader_path,
                                          spriv_shader_path)
        ret_value = subprocess.call(command, shell=True)

        if ret_value != 0:
            exit_with_msg("Error compiling shaders", ret_value)


def main(argv):
    if len(argv) != 3:
        exit_with_msg("Usage:\n"
                      "{} input_folder output_folder".format(argv[0]))

    input_folder = os.path.abspath(argv[1])
    output_folder = os.path.abspath(argv[2])
    if not os.path.exists(input_folder):
        exit_with_msg("Target folder does not exist", 1)

    print("================== Processing engine data ==================")
    remove_directory(output_folder)
    copy_directory(input_folder, output_folder)
    generate_spirv_shaders(output_folder)
    print("============================================================")


if __name__ == "__main__":
    main(sys.argv)
