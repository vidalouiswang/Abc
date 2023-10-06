Import("env", "projenv")
import platform
projectRootPath = env.get("PROJECT_DIR") + "/"

bootloaderPath = env.get("FLASH_EXTRA_IMAGES")[0][1]

osType = platform.system().lower()

if osType == "darwin":
    print("\nOS: MacOS\n")
    env.Replace( MKSPIFFSTOOL = projectRootPath + '/tools/mklittlefs' )
elif osType == "windows":
    print("\nOS: Windows\n")
    env.Replace( MKSPIFFSTOOL = projectRootPath + '/tools/mklittlefs.exe' )


env.Execute("node " + projectRootPath + "ap/tools.js " + projectRootPath)
env.Execute("node "+ projectRootPath + "scripts/replaceHtml.js " + projectRootPath)

def copyFirmware(source, target, env):
    env.Execute("node "+ projectRootPath +"scripts/copyFirmware.js " + bootloaderPath)

def OTA(source, target, env):
    env.Execute("node "+ projectRootPath +"scripts/autoOTA.js " + projectRootPath)

env.AddPostAction("buildprog", copyFirmware)
env.AddPostAction("upload", copyFirmware)

env.AddPostAction("buildprog", OTA)