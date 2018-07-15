import filecmp
import glob
import itertools
import os
import os.path
import re
import shutil
import stat

OPTION_FORCE = "force"
OPTION_VERBOSE = "verbose"
OPTION_PRESERVE = "preserve"
OPTION_NOOP = "noop"
OPTION_MODE = "mode"

STRING_CD_NOEXIST = "cd: no such file or directory: {}"

STRING_LN_ISDIRECTORY = "ln: {}: Is a directory"
STRING_LN_FILEEXIST = "ln: {}: File exists"
STRING_LN_VERBOSE = "ln: {} -> {}"

STRING_MV_NOEXIST = "mv: {}: No such file or directory"

STRING_CP_ISDIRECTORY = "cp: {} is a directory (not copied)."
STRING_CP_VERBOSE = "cp: {} -> {}"

STRING_RM_NOEXIST = "rm: {}: No such file or directory"
STRING_RM_ISDIRECTORY = "rm: {}: is a directory"
STRING_RM_VERBOSE = "rm: {}"

STRING_RMDIR_DIRNOTEMPTY = "rmdir: {}: Directory not empty"

STRING_CHOWN_NOEXIST = "chown: {}: No such file or directory"
STRING_CHOWN_ILLEGAL = "chown: {}:{}: illegal user:group name"
STRING_CHOWN_VERBOSE = "chown: {}"

STRING_CHMOD_NOEXIST = "chmod: {}: No such file or directory"
STRING_CHMOD_VERBOSE = "chmod: {}"

STRING_NOT_IMPLEMENTED = "NOT IMPLEMENTED"


class FileUtils:
    def copy_entry(src, dest, preserve=False,
                   dereference=False, verbose=False):
        if os.path.isdir(dest):
            FileUtils.remove_entry(dest)
        if os.path.isdir(src):
            def copy_fnc(src, dest):
                return FileUtils.copy_file(src, dest, preserve,
                                           dereference, verbose)
            shutil.copytree(src, dest, symlinks=dereference, ignore=None,
                            copy_function=copy_fnc)
        else:
            FileUtils.copy_file(src, dest, preserve, dereference)

    def copy_file(src, dest, preserve=False, dereference=True, verbose=False):
        if os.path.isdir(src):
            print(STRING_CP_ISDIRECTORY.format(src))
            return

        if preserve:
            shutil.copy2(src, dest, follow_symlinks=dereference)
        else:
            shutil.copy(src, dest, follow_symlinks=dereference)

        if verbose:
            print(STRING_CP_VERBOSE.format(src, dest))

    def remove_entry(path, force=False, verbose=False):
        if not os.path.lexists(path):
            if not force:
                print(STRING_RM_NOEXIST.format(path))
            return

        if os.path.isfile(path) or os.path.islink(path):
            FileUtils.remove_file(path, force, verbose)
        else:
            dirs = [path]
            for entry in os.listdir(path):
                entry_path = os.path.join(path, entry)
                FileUtils.remove_entry(entry_path, force, verbose)
                if os.path.isdir(entry_path) and not os.path.islink(entry_path):
                    dirs.append(entry_path)
            for entry in reversed(dirs):
                if verbose:
                    print(STRING_RM_VERBOSE.format(entry))
                FileUtils.rmdir(entry)

    def remove_entry_secure(path, force=False, verbose=False):
        print(STRING_NOT_IMPLEMENTED)

    def remove_file(path, force=False, verbose=False):
        if not os.path.lexists(path):
            if not force:
                print(STRING_RM_NOEXIST.format(path))
            return

        if os.path.isdir(path) and not os.path.islink(path):
            print(STRING_RM_ISDIRECTORY.format(path))
            return

        if os.path.islink(path):
            os.unlink(path)
        else:
            os.remove(path)
        if verbose:
            print(STRING_RM_VERBOSE.format(path))

    def compare_file(path_a, path_b):
        return filecmp.cmp(path_a, path_b)

    def uptodate(file, cmp_list):
        file_mtime = os.path.getmtime(file)
        for f in cmp_list:
            if file_mtime <= os.path.getmtime(f):
                return False
        return True

    def link(src, target, symbolic=False, force=False, verbose=False):
        if not symbolic and os.path.isdir(src):
            print(STRING_LN_ISDIRECTORY.format(src))
            return

        if os.path.isdir(target):
            target = os.path.join(target, os.path.basename(src))

        if os.path.isfile(target) or os.path.islink(target):
            if force:
                FileUtils.remove_file(target)
            else:
                print(STRING_LN_FILEEXIST.format(target))
                return

        if os.path.isdir(target):
            target = os.path.join(target, src)

        if symbolic:
            os.symlink(src, target)
        else:
            os.link(src, target)
        if verbose:
            print(STRING_LN_VERBOSE.format(target, src))

    #######################
    #  FileUtils from Ruby
    #######################

    def cd(path, **options):
        try:
            os.chdir(path)
        except FileNotFoundError:
            print(STRING_CD_NOEXIST.format(path))

    def pwd():
        print(os.getcwd())

    def mkdir(path, **options):
        try:
            os.mkdir(path, options.get(OPTION_MODE, 0o777))
        except OSError:
            pass

    def mkdir_p(path, **options):
        try:
            os.makedirs(path, options.get(OPTION_MODE, 0o777))
        except OSError:
            pass

    def rmdir(path, **options):
        try:
            os.rmdir(path)
        except OSError:
            print(STRING_RMDIR_DIRNOTEMPTY.format(path))

    def ln(src, target, **options):
        FileUtils.link(src, target, symbolic=False,
                       force=options.get(OPTION_FORCE, False),
                       verbose=options.get(OPTION_VERBOSE, False))

    def ln_s(src, target, **options):
        FileUtils.link(src, target, symbolic=True,
                       force=options.get(OPTION_FORCE, False),
                       verbose=options.get(OPTION_VERBOSE, False))

    def ln_sf(src, target, **options):
        options[OPTION_FORCE] = True
        FileUtils.ln_s(src, target, **options)

    def cp(src, dest, **options):
        verbose = options.get(OPTION_VERBOSE, False)

        FileUtils.copy_file(src, dest)

        if verbose:
            print(STRING_RM_VERBOSE.format(path))

    def cp_r(src, dest, **options):
        FileUtils.copy_entry(src, dest,
                             preserve=options.get(OPTION_PRESERVE, False),
                             verbose=options.get(OPTION_VERBOSE, False))

    def mv(src, dest, **options):
        verbose = options.get(OPTION_VERBOSE, False)
        force = options.get(OPTION_FORCE, False)

        if not os.path.exists(src):
            print(STRING_MV_NOEXIST.format(src))
            return

        if force:
            if os.path.isfile(dest):
                os.remove(dest)
            elif os.path.isdir(dest):
                dest_file = os.path.join(dest, os.path.basename(src))
                if os.path.isfile(dest_file):
                    os.remove(dest_file)
        shutil.move(src, dest)

    def rm(path, **options):
        FileUtils.remove_file(path,
                              force=options.get(OPTION_FORCE, False),
                              verbose=options.get(OPTION_VERBOSE, False))

    def rm_r(path, **options):
        FileUtils.remove_entry(path,
                               force=options.get(OPTION_FORCE, False),
                               verbose=options.get(OPTION_VERBOSE, False))

    def rm_rf(path, **options):
        options[OPTION_FORCE] = True
        FileUtils.rm_r(path, **options)

    def install(src, dest, mode, **options):
        print(STRING_NOT_IMPLEMENTED)

    def chmod(mode, path, **options):
        force = options.get(OPTION_FORCE, False)
        verbose = options.get(OPTION_VERBOSE, False)

        lol = {
            "ur": stat.S_IRUSR,
            "uw": stat.S_IWUSR,
            "ux": stat.S_IXUSR,
            "uX": stat.S_IXUSR,
            "us": stat.S_ISUID,
            "ut": None,
            "gr": stat.S_IRGRP,
            "gw": stat.S_IWGRP,
            "gx": stat.S_IXGRP,
            "gX": stat.S_IXGRP,
            "gs": stat.S_ISGID,
            "gt": None,
            "or": stat.S_IROTH,
            "ow": stat.S_IWOTH,
            "ox": stat.S_IXOTH,
            "oX": stat.S_IXOTH,
            "os": None,
            "ot": None,
            "ar": stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH,
            "aw": stat.S_IWUSR | stat.S_IWGRP | stat.S_IWOTH,
            "ax": stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH,
            "aX": stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH,
            "as": stat.S_ISUID | stat.S_ISGID,
            "at": stat.S_ISVTX,
        }

        result_mode = os.stat(path).st_mode

        regex = re.compile(r"^([ugoa]*)([\+\-\=])([rwxXst]+)$")
        matches = [re.match(regex, m) for m in mode.split(",")
                   if re.match(regex, m) is not None]
        for match in matches:
            classes = list(match.group(1))
            mode_operator = match.group(2)
            permissions = list(match.group(3))

            if len(classes) == 0:
                classes = ["a"]

            for c in classes:
                for p in permissions:
                    if os.path.isfile(path) and p == "X":
                        continue
                    if lol[c + p] is not None:
                        if mode_operator == "+":
                            result_mode |= lol[c + p]
                        elif mode_operator == "-":
                            result_mode &= ~lol[c + p]
                        elif mode_operator == "=":
                            pass  # TODO

        os.chmod(path, result_mode)

        if verbose:
            print(STRING_CHMOD_VERBOSE.format(mode + " " + path))

    def chmod_R(mode, path, **options):
        force = options.get(OPTION_FORCE, False)

        paths = []
        for dirpath, _, filenames in os.walk(path):
            paths.append(dirpath)
            paths += [os.path.join(dirpath, f) for f in filenames]

        if len(paths) == 0:
            if not force:
                print(STRING_CHMOD_NOEXIST.format(path))
            return

        for path in paths:
            FileUtils.chmod(mode, path, **options)

    def chown(user, group, path, **options):
        force = options.get(OPTION_FORCE, False)
        verbose = options.get(OPTION_VERBOSE, False)

        if not os.path.exists(path):
            if not force:
                print(STRING_CHOWN_NOEXIST.format(path))
            return

        try:
            shutil.chown(path, user, group)
        except LookupError as error:
            print(STRING_CHOWN_ILLEGAL.format(user, group))

        if verbose:
            print(STRING_CHOWN_VERBOSE.format(path))

    def chown_R(user, group, path, **options):
        force = options.get(OPTION_FORCE, False)

        paths = []
        for dirpath, _, filenames in os.walk(path):
            paths.append(dirpath)
            paths += [os.path.join(dirpath, f) for f in filenames]

        if len(paths) == 0:
            if not force:
                print(STRING_CHOWN_NOEXIST.format(path))
            return

        for path in paths:
            FileUtils.chown(user, group, path, **options)

    def touch(path, **options):
        with open(path, "w"):
            pass

    #######################
    #  Additional utils
    #######################

    def join(*args):
        return os.path.join(*args).replace("\\", "/")

    def which(name):
        return shutil.which(name)

    def glob(pathname, *, recursive=True):
        return glob.glob(pathname, recursive=recursive)

    def configure_file(input_file_path, output_file_path, config_dict):
        with open(input_file_path, "r") as input_file:
            with open(output_file_path, "w") as ouput_file:
                for line in input_file:
                    out_line = ""
                    matches = [it for it in re.finditer(r"@([\w_]+)@", line)]
                    start = end = 0
                    for match in matches:
                        key = match.group(1)
                        if key in config_dict:
                            end = match.start(0)
                            out_line += line[start:end] + config_dict[key]
                            start = match.end(0)
                        else:
                            print("Error, key @" + key +
                                  "@ not found. File: " + input_file_path)
                    out_line += line[start:]
                    ouput_file.write(out_line)

    def configure_directory(input_dir, output_dir,
                            config_dict, ignore_hidden=True):
        for dirpath, _, files in os.walk(input_dir):
            FileUtils.mkdir(os.path.join(output_dir,
                                         os.path.relpath(dirpath, input_dir)))
            for filename in files:
                if ignore_hidden and filename[0] == ".":
                    continue
                input_file_path = os.path.join(dirpath, filename)
                file_relpath = os.path.relpath(input_file_path, input_dir)
                base_relpath, extension = os.path.splitext(file_relpath)
                if extension == ".in":
                    output_file_path = os.path.join(output_dir, base_relpath)
                    FileUtils.configure_file(input_file_path,
                                             output_file_path, config_dict)
                else:
                    output_file_path = os.path.join(output_dir, file_relpath)
                    shutil.copy(input_file_path, output_file_path)
