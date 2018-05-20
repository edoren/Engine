import os
import argparse
import os.path
import sys
import urllib.request
from subprocess import run

from file_utils import FileUtils


VULKAN_LATEST_URLS = {
    "windows": "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe?Human=true",
    "linux": "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.run?Human=true",
    "macosx": "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan-sdk.tar.gz?Human=true"
}

CHUNK_SIZE = 1024


def download_for_linux(output_path):
    output_file = FileUtils.join(output_path, "vulkan_sdk.run")

    # Download the latest version of Vulkan
    url = VULKAN_LATEST_URLS["linux"]
    with open(output_file, "wb") as handle:
        response = urllib.request.urlopen(url)
        for chunk in iter(lambda: response.read(CHUNK_SIZE), ''):
            if not chunk:
                break
            handle.write(chunk)

    vulkan_sdk_folder = FileUtils.join(output_path, "VulkanSDK")
    extract_folder = FileUtils.join(output_path, "VulkanSDK_TMP")

    # Remote the old folders if they exist
    FileUtils.rm_rf(vulkan_sdk_folder)
    FileUtils.rm_rf(extract_folder, verbose=True)

    # Extract the file
    FileUtils.cd(output_path)
    run(["sh", output_file, "--quiet"])
    FileUtils.mv(vulkan_sdk_folder, extract_folder)
    FileUtils.rm(output_file)

    # Move the internal folder
    extract_folder_contents = os.listdir(extract_folder)
    if len(extract_folder_contents) == 1:
        vulkan_version = extract_folder_contents[0]
        internal_folder = FileUtils.join(extract_folder, vulkan_version)
        if os.path.isdir(internal_folder):
            FileUtils.mv(internal_folder, vulkan_sdk_folder)
    FileUtils.rmdir(extract_folder)

    if os.path.isdir(vulkan_sdk_folder):
        print("VulkanSDK version: {}".format(vulkan_version))
        print("VulkanSDK location: {}".format(vulkan_sdk_folder))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Download the latest VulkanSDK version"
    )
    parser.add_argument(
        "output",
        type=str,
        help="The output folder")

    args = parser.parse_args()

    if (sys.platform == "win32"):
        print("ERROR: Not implemented for Windows")
    elif (sys.platform == "linux"):
        download_for_linux(args.output)
    elif (sys.platform == "darwin"):
        print("ERROR: Not implemented for Mac")
        download_for_linux(args.output)
