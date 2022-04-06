import os
import subprocess
import platform
import Utils

from SetupPython import PythonConfiguration as PythonReq 

PythonReq.Validate()

from SetupPremake import PremakeConfiguration as PremakeReq
from SetupVulkan import VulkanConfiguration as VulkanReq

os.chdir('./../')

premakeInstalled = PremakeReq.Validate()
VulkanReq.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

print("\nUnzipping Module Libraries...")
Utils.UnzipFile("./Nebula/Modules/libs.zip", deleteZipFile=False)

if (premakeInstalled):
    if platform.system() == "Windows":
        print("\nRunning premake...")
        subprocess.call([os.path.abspath("./scripts/Win-GenProjects.bat"), "nopause"])

    print("\nSetup completed!")
else:
    print("Nebula requires Premake to generate project files.")