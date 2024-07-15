#! /usr/bin/env python3
import argparse, build, build_utils, common, glob, os, re, shutil, subprocess, sys
from typing import List, Tuple

def main() -> Tuple[str, str, str]:
  build.main()

  os.chdir(common.basedir)

  build_utils.copy_replace(
    "deploy/META-INF/maven/io.github.humbleui/jwm/pom.xml",
    "target/maven/META-INF/maven/io.github.humbleui/jwm/pom.xml",
    {"${version}": common.version}
  )

  build_utils.copy_replace(
    "deploy/META-INF/maven/io.github.humbleui/jwm/pom.properties",
    "target/maven/META-INF/maven/io.github.humbleui/jwm/pom.properties",
    {"${version}": common.version}
  )
  
  jar = build_utils.jar(f"target/jwm-{common.version}.jar", ("target/classes", "."), ("target/maven", "META-INF"))

  build_utils.makedirs("target/src/io/github/humbleui/jwm")
  shutil.copytree("x11/java", "target/src/io/github/humbleui/jwm", dirs_exist_ok=True)
  shutil.copytree("wayland/java", "target/src/io/github/humbleui/jwm", dirs_exist_ok=True)
  shutil.copytree("macos/java", "target/src/io/github/humbleui/jwm", dirs_exist_ok=True)
  shutil.copytree("shared/java", "target/src/io/github/humbleui/jwm", dirs_exist_ok=True)
  shutil.copytree("windows/java", "target/src/io/github/humbleui/jwm", dirs_exist_ok=True)
  sources = build_utils.jar(f"target/jwm-{common.version}-sources.jar", ("target/src", "."), ("target/maven", "META-INF"))

  build_utils.delombok(["target/src"], "target/delomboked", classpath=common.deps_compile())
  build_utils.javadoc(["target/delomboked"], "target/apidocs", classpath=common.deps_compile())
  javadoc = build_utils.jar(f"target/jwm-{common.version}-javadoc.jar", ("target/apidocs", "."), ("target/maven", "META-INF"))

  return (jar, sources, javadoc)

if __name__ == "__main__":
  main()
  sys.exit(0)
