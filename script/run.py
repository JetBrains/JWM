#! /usr/bin/env python3
import argparse, build, build_utils, common, glob, os, platform, subprocess, sys

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--example', default='dashboard')
  parser.add_argument('--jwm-version', default=None)
  parser.add_argument('--skija-version', default='0.116.2')
  parser.add_argument('--skija-dir', default=None)
  parser.add_argument('--skija-shared-jar', default=None)
  parser.add_argument('--skija-platform-jar', default=None)
  parser.add_argument('--types-dir', default=None)
  parser.add_argument('--just-run', action='store_true')
  args = parser.parse_args()

  if not args.jwm_version and not args.just_run:
    build.main()

  if args.skija_dir:
    skija_dir = os.path.abspath(args.skija_dir)

  if args.skija_platform_jar:
    skija_platform_jar = os.path.abspath(args.skija_platform_jar)
  
  os.chdir(common.basedir)

  # compile
  classpath = common.deps_compile()
  
  if args.jwm_version:
    classpath += [
      build_utils.fetch_maven('io.github.humbleui.jwm', 'jwm', args.jwm_version)
    ]
  else:
    classpath += [
          'target/classes'
        ]
    if build_utils.system == "linux":
        classpath += [
          "wayland/build",
          "x11/build"
        ]
    else:
      classpath += [
        build_utils.system + '/build'
      ]

  if args.skija_dir:
    classpath += [
      skija_dir + '/platform/target/' + build_utils.system + '-' + build_utils.arch + '/classes',
    ]
  elif args.skija_platform_jar:
    classpath += [
      skija_platform_jar
    ]
  else:
    skija_native = 'skija-' + build_utils.system + '-' + build_utils.arch
    classpath += [
      build_utils.fetch_maven('io.github.humbleui', skija_native, args.skija_version),
    ]
  sources = build_utils.files(f'examples/{args.example}/java/**/*.java')
  build_utils.javac(sources, f'examples/{args.example}/target/classes', classpath = classpath, release='16')
  
  # run
  subprocess.check_call([
    'java',
    '--class-path', build_utils.classpath_join(classpath + [f'examples/{args.example}/target/classes']),
    '-Djava.awt.headless=true',
    '-enableassertions',
    '-enablesystemassertions',
    '-Dfile.encoding=UTF-8',
    '-Xcheck:jni',
    'io.github.humbleui.jwm.examples.Example'
  ])

  return 0

if __name__ == '__main__':
  sys.exit(main())
