# Originally copied from https://github.com/bbreslauer/qt-bazel-example

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
      cmd = "/Users/rjryan/.homebrew/Cellar/qt/5.10.1/bin/uic $(locations %s) -o $@" % ui,
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
    cmd = "/Users/rjryan/.homebrew/Cellar/qt/5.10.1/bin/rcc $(location %s) -o $@" % src,
  )

  native.cc_library(
    name = name,
    srcs = ["qrc_%s.cpp" % name],
    deps = ["@Qt5Core//:lib"],
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
      cmd = "/Users/rjryan/.homebrew/Cellar/qt/5.10.1/bin/moc $(location %s) -o $@ -f'%s'" % (hdr, hdr),
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
      cmd =  "/Users/rjryan/.homebrew/Cellar/qt/5.10.1/bin/moc $(location %s) -o $@ -f'%s'" \
        % (hdr, '%s/%s' % (PACKAGE_NAME, hdr)),
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
