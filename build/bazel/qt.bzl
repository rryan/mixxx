# Originally copied from https://github.com/bbreslauer/qt-bazel-example

load(":pkg_config.bzl", "find_pkgconfig", "get_cflags", "get_linkopts", "extract_includes", "extract_defines")

def _impl(ctx):
  which_moc = ctx.which('moc')
  qtdir = ctx.os.environ.get('QTDIR')
  if qtdir:
    print('Found Qt via $QTDIR:', qtdir)
  elif which_moc:
    # Get prefix from bin/moc
    print('Found Qt via moc:', which_moc)
    qtdir = ctx.path(which_moc).dirname.dirname

  qtdir_path = ctx.path(qtdir)
  bin_path = qtdir_path.get_child('bin')

  moc_path = bin_path.get_child('moc')
  uic_path = bin_path.get_child('uic')
  rcc_path = bin_path.get_child('rcc')

  ctx.symlink(qtdir, 'qt')

  libraries = ""

  lib_path = qtdir_path.get_child('lib')
  pkgconfig_path = lib_path.get_child('pkgconfig')

  pkg_config = find_pkgconfig(ctx)

  blacklist = ["Qt5Designer"]
  qt_modules = [pc_file.basename.split('.')[0] for pc_file in pkgconfig_path.readdir()]
  qt_modules = [mod for mod in qt_modules if mod not in blacklist]

  include_path = qtdir_path.get_child('include')
  includes = ', '.join(['"qt/include/%s"' % subdir.basename for subdir in include_path.readdir()])

  for module in qt_modules:
    pc_args = [pkg_config.value, module]

    cflags = get_cflags(ctx, pc_args)
    _, cflags = extract_includes(cflags.value)
    defines, cflags = extract_defines(cflags)
    cflags = ' '.join([str(flag) for flag in cflags])
    linkopts = get_linkopts(ctx, pc_args)
    linkopts = ' '.join([str(opt) for opt in linkopts.value])
    defines = ', '.join(['"%s"' % define for define in defines])

    libraries += (
"""cc_library(
  name = "%s",
  srcs = [],
  copts = ["%s"],
  defines = [%s],
  linkopts = ["%s"],
  deps = [":headers"],
)
""" % (module, cflags, defines, linkopts))
  ctx.template("BUILD.bazel", Label("//build/bazel:BUILD.qt"), {
    "%{libraries}": libraries,
    "%{includes}": includes,
  })


_qt5_repository_rule = repository_rule(
  implementation=_impl,
  attrs={

  },
  environ = ["QTDIR"],
)

def qt5_package():
  _qt5_repository_rule(name="qt5")


def qt_ui_library(name, ui, deps):
  """Compiles a QT UI file and makes a library for it.

  Args:
    name: A name for the rule.
    src: The ui file to compile.
    deps: cc_library dependencies for the library.
  """
  ui_path = '/'.join(ui.split('/')[:-1])
  ui_filename = ui.split('/')[-1]
  dest_path = "%s/ui_%s.h" % (ui_path, ui_filename.split('.')[0])

  native.genrule(
      name = "%s_uic" % name,
      srcs = [ui],
      outs = [dest_path],
      cmd = "$(location @qt5//:uic) $(locations %s) -o $@" % ui,
      tools = ["@qt5//:uic"],
  )

  native.cc_library(
      name = name,
      hdrs = [":%s_uic" % name],
      #include_prefix = ui_path,
      deps = deps,
      #copts = ["-Isrc"],
  )

def qt_qrc_library(name, src, data, **kwargs):
  native.genrule(
    name = "%s_qrc" % name,
    srcs = [src] + data,
    outs = ["qrc_%s.cpp" % name],
    cmd = "$(location @qt5//:rcc) $(location %s) -o $@" % src,
    tools = ["@qt5//:rcc"],
  )

  native.cc_library(
    name = name,
    srcs = ["qrc_%s.cpp" % name],
    deps = ["@qt5//:Qt5Core"],
    alwayslink = 1,
    **kwargs)

def qt_moc_header(hdr):
  hdr_path = '/'.join(hdr.split('/')[:-1])
  hdr_filename = hdr.split('/')[-1]
  if hdr_path:
    dest_path = "%s/moc_%s.cpp" % (hdr_path, hdr_filename.split('.')[0])
  else:
    dest_path = "moc_%s.cpp" % (hdr_filename.split('.')[0])
  native.genrule(
      name = "%s_moc" % hdr.replace('/', '_'),
      srcs = [hdr],
      outs = [dest_path],
      # cmd =  "/Users/rjryan/.homebrew/Cellar/qt/5.10.1/bin/moc $(location %s) -o $@ -f'%s'" \
      #   % (hdr, '%s/%s' % (PACKAGE_NAME, hdr)),
      cmd = "$(location @qt5//:moc) $(location %s) -o $@ -f'%s'" % (hdr, hdr),
      tools = ["@qt5//:moc"],
  )
  return ":%s_moc" % hdr.replace('/', '_')


def qt_cc_library(name, src, hdr, normal_hdrs=[], deps=None, ui=None,
                  ui_deps=None, **kwargs):
  """Compiles a QT library and generates the MOC for it.

  If a UI file is provided, then it is also compiled with UIC.

  Args:
    name: A name for the rule.
    src: The cpp file to compile.
    hdr: The single header file that the MOC compiles to src.
    normal_hdrs: Headers which are not sources for generated code.
    deps: cc_library dependencies for the library.
    ui: If provided, a UI file to compile with UIC.
    ui_deps: Dependencies for the UI file.
    kwargs: Any additional arguments are passed to the cc_library rule.
  """
  hdr_path = '/'.join(hdr.split('/')[:-1])
  hdr_filename = hdr.split('/')[-1]
  dest_path = "%s/moc_%s.cpp" % (hdr_path, hdr_filename.split('.')[0])
  native.genrule(
      name = "%s_moc" % name,
      srcs = [hdr],
      outs = [dest_path],
      cmd =  "$(location @qt5//:moc) $(location %s) -o $@ -f'%s'" \
        % (hdr, '%s/%s' % (PACKAGE_NAME, hdr)),
      tools = ["@qt5//:moc"],
  )
  srcs = [src, ":%s_moc" % name]

  if ui:
    qt_ui_library("%s_ui" % name, ui, deps=ui_deps)
    deps.append("%s_ui" % name)

  hdrs = [hdr] + normal_hdrs

  native.cc_library(
      name = name,
      srcs = srcs,
      hdrs = hdrs,
      deps = deps,
      **kwargs
  )
