HOMEBREW_PATH = "/Users/rjryan/.homebrew"

# def homebrew_repository(name, version=None, package_name=None):
#   if not package_name:
#     package_name = name
#   cellar_path = '%s/Cellar/%s' % (HOMEBREW_PATH, name)
#   # if not version:
#   #   versions = native.glob([cellar_path + '/*'])
#   #   print(versions)
#   #   version = versions[0]
#   native.new_local_repository(
#     name = package_name,
#     path = cellar_path, #"%s/Cellar/%s/" % (HOMEBREW_PATH, name),
#     build_file = "BUILD.%s" % name
#   )

# load(
#     "@bazel_tools//tools/build_defs/repo:utils.bzl",
#     "workspace_and_buildfile",
# )

def _impl(ctx):
  #homebrew_path = ctx.os.environ['HOMEBREW_PATH']
  #print('Homebrew: ', homebrew_path)
  cellar_path = ctx.path(HOMEBREW_PATH + '/Cellar/' + ctx.attr.homebrew_package_name)
  if not cellar_path.exists:
    ctx.fail("HOMEBREW_PATH doesn't exist")
  versions = cellar_path.readdir()

  print("Homebrew %s: %s" % (ctx.name, versions))

  if len(versions) != 1:
    ctx.fail("Multiple versions for %s: %s" % (ctx.name, versions))

  ctx.symlink(versions[0], ctx.attr.homebrew_package_name)

  #workspace_and_buildfile(ctx)
  if ctx.attr.build_file:
    bash_exe = ctx.os.environ["BAZEL_SH"] if "BAZEL_SH" in ctx.os.environ else "bash"
    ctx.execute([bash_exe, "-c", "rm -f BUILD BUILD.bazel"])
    ctx.symlink(ctx.attr.build_file, "BUILD.bazel")
  elif ctx.attr.build_file_content:
    bash_exe = ctx.os.environ["BAZEL_SH"] if "BAZEL_SH" in ctx.os.environ else "bash"
    ctx.execute([bash_exe, "-c", "rm -f BUILD.bazel"])
    ctx.file("BUILD.bazel", ctx.attr.build_file_content)


  #ctx.file('WORKSPACE', "workspace(name = \"{name}\")\n".format(name=ctx.name))
  #print(ctx.attr.build_file)
  #print(str(ctx.path(ctx.attr.build_file)))
  #print(ctx.attr.build_file)
  #ctx.symlink(ctx.path(ctx.attr.build_file),
  #                       'BUILD.bazel')

  #result = ctx.execute(["echo", "/tmp/dummy/path"])
  #llvm_path = result.stdout.splitlines()[0]
  #ctx.symlink(llvm_path, "llvm-4.0")
  #ctx.file("BUILD", """
  # filegroup(
  #     name = "llvm_files",
  #     srcs = glob(["llvm-4.0/**"]),
  #     visibility = ["//visibility:public"],
  # )
  # """)

_homebrew_repository = repository_rule(
    implementation=_impl,
    local = True,
    attrs={
      "homebrew_package_name": attr.string(mandatory=True),
      "build_file": attr.label(),
      "build_file_content": attr.string(),
      "workspace_file": attr.label(),
      "workspace_file_content": attr.string(),
      "libraries": attr.string_list(),
      "binaries": attr.string_list(),
    },
    environ = ["HOMEBREW_PATH"],
)

def homebrew_repository(name, package_name=None, build_file=None, libraries=None):
  if not package_name:
    package_name = name

  if not libraries:
    libraries = []


  _homebrew_repository(
    name = package_name,
    homebrew_package_name = name,
    build_file=build_file,
  )
