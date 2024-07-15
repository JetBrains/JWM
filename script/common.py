#! /usr/bin/env python3
import argparse, build_utils, functools, os

basedir = os.path.abspath(os.path.dirname(__file__) + '/..')

@functools.lru_cache(maxsize=1)
def deps_compile():
  parser = argparse.ArgumentParser()
  parser.add_argument('--skija-dir', default=None)
  parser.add_argument('--skija-shared-jar', default=None)
  parser.add_argument('--skija-version', default='0.116.2')
  (args, _) = parser.parse_known_args()

  deps = [
    build_utils.fetch_maven('org.projectlombok', 'lombok', '1.18.30'),
    build_utils.fetch_maven('org.jetbrains', 'annotations', '20.1.0')
  ]

  if args.skija_dir:
    deps += [
      build_utils.execdir + '/' + args.skija_dir + '/shared/target/classes-java9',
      build_utils.execdir + '/' + args.skija_dir + '/shared/target/classes',
    ]
  elif args.skija_shared_jar:
    deps += [
      build_utils.execdir + '/' + args.skija_shared_jar
    ]
  else:
    deps += [
      build_utils.fetch_maven('io.github.humbleui', 'skija-shared', args.skija_version),
    ]

  types_dir = build_utils.get_arg("types-dir")
  if types_dir:
    deps.append(os.path.normpath(build_utils.execdir + "/" + types_dir + "/target/classes"))
  else:
    deps.append(build_utils.fetch_maven('io.github.humbleui', 'types', '0.2.0'))

  return deps

version = build_utils.get_arg("version") or build_utils.parse_ref() or build_utils.parse_sha() or "0.0.0-SNAPSHOT"
