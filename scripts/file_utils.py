
import errno
import os
import os.path
import re
import shutil


def create_directory(path):
    try:
        os.makedirs(path)
        return True
    except OSError:
        return os.path.isdir(path)


def move_file(src, dst, force=False):
    if force:
        if os.path.isfile(dst):
            os.remove(dst)
        elif os.path.isdir(dst):
            dst_file = os.path.join(dst, os.path.basename(src))
            if os.path.isfile(dst_file):
                os.remove(dst_file)
    shutil.move(src, dst)


def copy_directory(src, dst, symlinks=False, ignore=None, force=False):
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            if force:
                remove_directory(d)
            shutil.copytree(s, d, symlinks, ignore)


def remove_directory(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)


def configure_file(input_file_path, output_file_path, config_dict):
    with open(input_file_path, "r") as input_file:
        with open(output_file_path, "w") as ouput_file:
            for line in input_file:
                matches = [it for it in re.finditer(r"@([\w_]+)@", line)]
                for match in matches:
                    key = match.group(1)
                    if key in config_dict:
                        start = match.start(0)
                        end = match.end(0)
                        line = line[:start] + config_dict[key] + line[end:]
                    else:
                        print("Error, key @" + key +
                              "@ not found. File: " + input_file_path)
                ouput_file.write(line)


def configure_directory(input_dir, output_dir,
                        config_dict, ignore_hidden=True):
    for dirpath, _, files in os.walk(input_dir):
        create_directory(os.path.join(output_dir,
                                      os.path.relpath(dirpath, input_dir)))
        for filename in files:
            if (ignore_hidden and filename[0] == "."):
                continue
            input_file_path = os.path.join(dirpath, filename)
            file_relpath = os.path.relpath(input_file_path, input_dir)
            base_relpath, extension = os.path.splitext(file_relpath)
            if extension == ".in":
                output_file_path = os.path.join(output_dir, base_relpath)
                configure_file(input_file_path, output_file_path, config_dict)
            else:
                output_file_path = os.path.join(output_dir, file_relpath)
                shutil.copy(input_file_path, output_file_path)
