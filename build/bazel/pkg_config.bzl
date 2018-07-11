"""Skylark module for system libraries known to pkg-config.
Copied from https://github.com/bowlofstew/bazel-boost
pkg_config_module(name, modname,
                  atleast_version, exact_version, max_version)
  Create a local repository based on the results of pkg-config.
  Args:
    name: string. The name of the repository.
    modname: string. The name of the pkg-config module.
    atleast_version: Optional, string. Require at least this version.
    exact_version: Optional, string. Require exactly this version.
    max_version: Optional, string.  Require less than this version.
  The configured repository will have a `cc_library` target with the
  provided `modname` as well as an alias to `lib`.
"""

load(":common.bzl", "error", "success", "write_build")
load(":wrapped_ctx.bzl", "unwrap")

def _write_build(repo_ctx, cflags, include_dir, linkopts):
  includes, defines = _parse_cflags(repo_ctx, cflags, include_dir)
  write_build(repo_ctx, "lib", includes, defines, linkopts)

def _fail(repo_ctx, message, tail=""):
  """Fail with message if repo_ctx.attr.mandatory, otherwise warn."""
  modname = repo_ctx.attr.modname
  if tail:
    message = "\nError processing {}: {}\n\n{}".format(modname, message, tail)
  else:
    message = "\nError processing {}: {}".format(modname, message)
  return error(message)

def find_pkgconfig(repo_ctx):
  pkg_config = unwrap(repo_ctx).which("pkg-config")
  if pkg_config == None:
    return _fail(repo_ctx, "Unable to find pkg-config executable")
  return success(pkg_config)

def _exists(repo_ctx, pc_args):
  result = unwrap(repo_ctx).execute(pc_args + ["--print-errors", "--exists"])
  if result.return_code != 0:
    return _fail(repo_ctx, "Unable to find module", result.stderr)
  return success(True)

def _installed_version(repo_ctx, pc_args):
  result = unwrap(repo_ctx).execute(pc_args + ["--modversion"])
  if result.return_code != 0:
    return _fail(repo_ctx, "Unable to determine installed version", result.stderr)
  return success(result.stdout.strip())

def _check_version(repo_ctx, pc_args):
  version_args = []
  for name in ["atleast_version", "max_version", "exact_version"]:
    value = getattr(repo_ctx.attr, name, "")
    if value:
      version_args += ["--{}={}".format(name.replace("_", "-"), value)]
  if not version_args:
    return success(True)

  result = unwrap(repo_ctx).execute(pc_args + version_args)
  if result.return_code != 0:
    version = _installed_version(repo_ctx, pc_args)
    if version.error != None:
      return version
    return _fail(repo_ctx,
                 "Installed version {} doesn't match constraints: {}".format(
                     version.value, " ".join(version_args)),
                 result.stderr)
  else:
    return success(True)

def get_cflags(repo_ctx, pc_args):
  result = unwrap(repo_ctx).execute(pc_args + ["--cflags"])
  if result.return_code != 0:
    return _fail(repo_ctx, "Unable to determine cflags", result.stderr)
  stdout = result.stdout
  return success([arg for arg in stdout.strip().split(" ") if arg])

def _include_dir(repo_ctx, pc_args):
  result = unwrap(repo_ctx).execute(pc_args + ["--variable=includedir"])
  if result.return_code != 0:
    return success([])
  stdout = result.stdout
  return success([arg for arg in stdout.strip().split(" ") if arg])

def get_linkopts(repo_ctx, pc_args):
  result = unwrap(repo_ctx).execute(pc_args + ["--static", "--libs"])
  if result.return_code != 0:
    return _fail(repo_ctx, "Unable to determine linkopts", result.stderr)
  stdout = result.stdout
  return success([' '.join([arg for arg in stdout.strip().split(" ") if arg])])

def _extract_prefix(flags, prefix):
  stripped, remain = [], []
  for arg in flags:
    if arg.startswith(prefix):
      stripped += [arg[len(prefix):]]
    else:
      remain += [arg]
  return stripped, remain

def extract_includes(cflags):
  return _extract_prefix(cflags, "-I")

def extract_defines(cflags):
  return _extract_prefix(cflags, "-D")

def _symlink_directories(repo_ctx, basename, pathnames):
  result = []
  root = unwrap(repo_ctx).path("")
  base = root.get_child(basename)
  rootlen = len(str(base)) - len(basename)  # Include separator length.
  pathnames = depset(pathnames)
  for idx, srcpath in enumerate([unwrap(repo_ctx).path(p) for p in pathnames]):
    basename = '%s%d' % (srcpath.basename, idx)
    basename = str(srcpath).replace('/', '_')
    destpath = base.get_child(basename)
    unwrap(repo_ctx).symlink(srcpath, destpath)
    result += [str(destpath)[rootlen:]]
  return result

def _parse_cflags(repo_ctx, cflags, include_dir):
  includes, cflags = extract_includes(cflags)
  defines, cflags = extract_defines(cflags)
  if cflags:
    print("In pkg-config module {}, unhandled cflags: {}".format(
          repo_ctx.attr.modname, cflags))
  if include_dir:
    includes.extend(include_dir)

  # We can only reliably point to include directories.
  # `linkopts` leak into runtime for dynamic libraries and
  # should point to the system paths.
  includes = _symlink_directories(repo_ctx, "include", includes)
  return includes, defines

def setup_pkg_config_package(repo_ctx):
  pkg_config = find_pkgconfig(repo_ctx)
  if pkg_config.error != None:
    return pkg_config
  pc_args = [pkg_config.value, repo_ctx.attr.modname]
  exists = _exists(repo_ctx, pc_args)
  if exists.error != None:
    return exists
  version = _check_version(repo_ctx, pc_args)
  if version.error != None:
    return version
  cflags = get_cflags(repo_ctx, pc_args)
  if cflags.error != None:
    return cflags

  include_dir = _include_dir(repo_ctx, pc_args)
  if include_dir.error != None:
    return include_dir

  linkopts = get_linkopts(repo_ctx, pc_args)
  if linkopts.error != None:
    return linkopts

  _write_build(repo_ctx, cflags.value, include_dir.value, linkopts.value)
  return success(True)

def _impl(repo_ctx):
  print(repo_ctx.name)
  error = setup_pkg_config_package(repo_ctx).error
  if error != None:
    fail(error)

pkg_config_package = repository_rule(
    _impl,
    attrs = {
        "modname": attr.string(mandatory = True),
        "atleast_version": attr.string(),
        "max_version": attr.string(),
        "exact_version": attr.string(),
        "build_file_template": attr.label(
            default = Label("//build/bazel:BUILD.tpl"),
            single_file = True,
            allow_files = True,
        ),
    },
    local = True,
)
