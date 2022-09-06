Import("env", "projenv")
import platform
projectRootPath = env.get("PROJECT_DIR") + "/"

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
    env.Execute("node "+ projectRootPath +"scripts/copyFirmware.js " + projectRootPath)

def OTA(source, target, enc):
    env.Execute("node "+ projectRootPath +"scripts/autoOTA.js " + projectRootPath)

env.AddPostAction("buildprog", copyFirmware)

#env.AddPostAction("buildprog", OTA)
env.AddPostAction("upload", copyFirmware)