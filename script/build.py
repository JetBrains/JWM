#! /usr/bin/env python3
import argparse, build_utils, common, glob, os, platform, subprocess, sys

def build_native_system(system):
  os.chdir(common.basedir + "/" + system)
  subprocess.check_call(["cmake",
    "-DCMAKE_BUILD_TYPE=Release",
    "-B", "build",
    "-G", "Ninja",
    "-DJWM_ARCH=" + build_utils.arch,
    *(["-DCMAKE_OSX_ARCHITECTURES=" + {"x64": "x86_64", "arm64": "arm64"}[build_utils.arch]] if build_utils.system == "macos" else [])])
  subprocess.check_call(["ninja"], cwd = "build")
  
  if os.path.exists('build/libjwm_x64.dylib'):
    build_utils.copy_newer('build/libjwm_x64.dylib', '../target/classes/libjwm_x64.dylib')
  
  if os.path.exists('build/libjwm_arm64.dylib'):
    build_utils.copy_newer('build/libjwm_arm64.dylib', '../target/classes/libjwm_arm64.dylib')
  
  if os.path.exists('build/libjwm_x64.so'):
    build_utils.copy_newer('build/libjwm_x64.so', '../target/classes/libjwm_x64.so')
  
  if os.path.exists('build/jwm_x64.dll'):
    build_utils.copy_newer('build/jwm_x64.dll', '../target/classes/jwm_x64.dll')

  return 0

def build_native():
  # TODO: There is likely a better folder hierarchy than this.
  cur_system = build_utils.system;
  if cur_system == "linux":
    build_native_system("x11")
    build_native_system("wayland")
  else:
    build_native_system(cur_system)

  return 0



def build_java():
  os.chdir(common.basedir)
  sources = build_utils.files("x11/java/**/*.java", "wayland/java/**/*.java", "macos/java/**/*.java", "shared/java/**/*.java",  "windows/java/**/*.java",)
  build_utils.javac(sources, "target/classes", classpath=common.deps_compile())
  return 0

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--only', choices = ['native', 'java'])
  (args, _) = parser.parse_known_args()
  res = 0
  if None == args.only or 'native' == args.only:
    res += build_native()
  if None == args.only or 'java' == args.only:
    res += build_java()
  return res

if __name__ == '__main__':
  sys.exit(main())
