from SCons import Script
import datetime
import os
import os.path
import re
import stat
import time

CURRENT_VCS = None


def get_current_vcs():
    if CURRENT_VCS is not None:
        return CURRENT_VCS
    if on_git():
        return "git"
    return "tar"


def on_git():
    cwd = os.getcwd()
    basename = " "
    while len(basename) > 0:
        try:
            os.stat(os.path.join(cwd, ".git"))
            return True
        except:
            pass
        cwd, basename = os.path.split(cwd)
    return False


def get_revision():
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return get_git_revision()
    if CURRENT_VCS == "tar":
        return ""
    return None


def get_modified():
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return get_git_modified()
    if CURRENT_VCS == "tar":
        return ""
    return None


def get_branch_name():
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return get_git_branch_name()
    if CURRENT_VCS == "tar":
        return ""
    return None


def export_source(source, dest):
    global CURRENT_VCS
    if CURRENT_VCS is None:
        CURRENT_VCS = get_current_vcs()
    if CURRENT_VCS == "git":
        return export_git(source, dest)
    return None


def get_git_revision():
    return len(os.popen("git log --pretty=oneline --first-parent").read().splitlines())


def get_git_modified():
    modified_matcher = re.compile(
        "^#.*:   (?P<filename>.*?)$")  # note output might be translated
    modified_files = []
    for line in os.popen("git status").read().splitlines():
        match = modified_matcher.match(line)
        if match:
            match = match.groupdict()
            modified_files.append(match['filename'].strip())
    return "\n".join(modified_files)


def get_git_branch_name():
    # this returns the branch name or 'HEAD' in case of detached HEAD
    branch_name = os.popen(
        "git rev-parse --abbrev-ref HEAD").readline().strip()
    if branch_name == 'HEAD':
        # Use APPVEYOR_REPO_BRANCH variable if building on appveyor or (no branch) if unset
        branch_name = os.getenv("APPVEYOR_REPO_BRANCH", '(no branch)')
    # Add PR# to branch name if building a PR in appveyor to avoid package naming collision
    PRnum = os.getenv("APPVEYOR_PULL_REQUEST_NUMBER")
    if PRnum != None:
        branch_name += ("-PR" + PRnum)
    return branch_name


def export_git(source, dest):
    os.mkdir(dest)
    return os.system('git archive --format tar HEAD %s | tar x -C %s' % (source, dest))


def get_build_dir(platformString, bitwidth):
    build_dir = '%s%s_build' % (platformString[0:3], bitwidth)
    return build_dir


def get_mixxx_version():
    """Get Mixxx version number from defs_version.h"""
    # have to handle out-of-tree building, that's why the '#' :(
    defs = Script.File('#src/defs_version.h')
    version = ""

    for line in open(str(defs)).readlines():
        if line.strip().startswith("#define MIXXX_VERSION "):
            version = line
            break

    if version == "":
        raise ValueError("Version not found")

    version = version.split()[-1].replace('"', '')

    # Check if version respect constraints
    # 3 numbers separated by a dot, then maybe a (dash or tilde) and some string
    # See src/defs_version.h comment
    versionMask = '^\d+\.\d+\.\d+([-~].+)?$'
    if not re.match(versionMask, version):
        raise ValueError("Version format mismatch. See src/defs_version.h comment")

    return version


def get_flags(env, argflag, default=0):
    """
    * get value passed as an argument to scons as argflag=value
    * if no value is passed to scons use stored value
    * if no value is stored, use default
    Returns the value and stores it in env[argflag]
    """
    flags = Script.ARGUMENTS.get(argflag, -1)
    if int(flags) < 0:
        if argflag in env:
            flags = env[argflag]
        else:  # default value
            flags = default
    env[argflag] = flags
    return flags


# Checks for pkg-config on Linux
def CheckForPKGConfig(context, version='0.0.0'):
    context.Message(
        "Checking for pkg-config (at least version %s)... " % version)
    ret = context.TryAction(
        "pkg-config --atleast-pkgconfig-version=%s" % version)[0]
    context.Result(ret)
    return ret


# Uses pkg-config to check for a minimum version
def CheckForPKG(context, name, version=""):
    if version == "":
        context.Message("Checking for %s... \t" % name)
        ret = context.TryAction("pkg-config --exists '%s'" % name)[0]
    else:
        context.Message(
            "Checking for %s (%s or higher)... \t" % (name, version))
        ret = context.TryAction(
            "pkg-config --atleast-version=%s '%s'" % (version, name))[0]
        context.Result(ret)
    return ret


def write_build_header(path):
    f = open(path, 'w')
    try:
        branch_name = get_branch_name()
        modified = len(get_modified()) > 0
        # Do not emit BUILD_BRANCH on release branches.
        if branch_name and not branch_name.startswith('release'):
            f.write('#define BUILD_BRANCH "%s"\n' % branch_name)
        f.write('#define BUILD_REV "%s%s"\n' % (get_revision(),
                                                '+' if modified else ''))
    finally:
        f.close()
        os.chmod(path, stat.S_IRWXU | stat.S_IRWXG |stat.S_IRWXO)

def get_osx_min_version():
    """Gets the minimum required OS X version from product_definition.plist."""
    # Mixxx 2.0 supported OS X 10.6 and up.
    # Mixxx >2.0 requires C++11 which is only available with libc++ and OS X
    # 10.7 onwards. std::promise/std::future requires OS X 10.8.
    # Mixxx >2.2 switched to Qt 5, which requires macOS 10.11.
    # Use SCons to get the path relative to the repository root.
    product_definition = str(Script.File('#build/osx/product_definition.plist'))
    p = os.popen("/usr/libexec/PlistBuddy -c 'Print os:0' %s" % product_definition)
    min_version = p.readline().strip()
    result_code = p.close()
    assert result_code is None, "Can't read macOS min version: %s" % min_version
    return min_version


def write_rc_include_header(mixxx_version, vcs_revision, is_debug):
    """Writes src/mixxx.rc.include."""
    with open(File('#src/mixxx.rc.include').abspath, "w") as f:
        str_list = []
        str_list.append('#define VER_FILEVERSION             ')
        # Remove anything after ~ or - in the version number and replace the
        # dots with commas.
        str_list.append(mixxx_version.partition('~')[0].partition('-')[0].replace('.',','))
        if vcs_revision:
            str_list.append(',' + str(vcs_revision))
        str_list.append('\n')

        str_list.append('#define VER_PRODUCTVERSION          ')
        str_list.append(mixxx_version.partition('~')[0].partition('-')[0].replace('.',','))
        if vcs_revision:
            str_list.append(','+str(vcs_revision))
        str_list.append('\n')

        now = datetime.datetime.now()
        str_list.append('#define CUR_YEAR                    "' + str(now.year) + '"\n\n')

        if is_debug:
            str_list.append('#define DEBUG                       1\n')
        if 'pre' in mixxx_version.lower():
            str_list.append('#define PRERELEASE                  1\n')

        fo.write(''.join(str_list))


def ubuntu_append_changelog(debian_dir,
                            package_name, package_version,
                            description,
                            distro='lucid',
                            urgency='low',
                            author="Mixxx Buildbot <builds@mixxx.org>"):
    now_formatted = time.strftime("%a,  %d %b %Y %H:%M:%S +0000", time.gmtime())
    new_entry = [
        "%s (%s) %s; urgency=%s" % (package_name, package_version, distro, urgency),
        "",
        description,
        "",
        " -- %s  %s" % (author, now_formatted),
        "",
    ]
    lines = []
    with open(os.path.join(debian_dir, 'changelog'), 'r') as changelog:
        lines = list(changelog)
    with open(os.path.join(debian_dir, 'changelog'), 'w') as changelog:
        changelog.writelines(["%s\n" % x for x in new_entry])
        changelog.writelines(lines)


def ubuntu_cleanup():
    os.system('rm -rf ubuntu')
    os.mkdir('ubuntu')


def construct_version(build, mixxx_version, branch_name, vcs_revision):
    if branch_name.startswith('release-'):
        branch_name = branch_name.replace('release-', '')

    # Include build type in the filename.
    build_type = 'release' if build.build_is_release else 'debug'

    # New, simpler logic: mixxx version, branch name, git revision,
    # release/build. Example: mixxx-1.12.0-master-gitXXXX-release
    return "%s-%s-%s%s-%s" % (mixxx_version, branch_name, vcs_name,
                              vcs_revision, build_type)


def ubuntu_construct_version(build, mixxx_version, branch_name, vcs_revision,
                             ubuntu_version, distro_version):
    # The format of a Debian/Ubuntu version is:
    #
    #   [epoch:]upstream_version[-debian_revision]
    #
    # A detailed description of the valid characters and sorting order of
    # versions can be found here:
    # https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Version
    #
    # For package upgrades to work correctly, we want the following
    # orderings on package versions:
    #
    # nightly build < pre-alpha < alpha < beta < rc1 < rc2 < final release
    #
    # The sorting rules are complicated but the key detail is: "The lexical
    # comparison is a comparison of ASCII values modified so that all the
    # letters sort earlier than all the non-letters and so that a tilde
    # sorts before anything, even the end of a part."
    #
    # The Mixxx version stored in src/defs_version.h (the "mixxx_version"
    # parameter to this function) is formatted like:
    #
    # Pre Alpha: 2.0.0-alpha-pre
    # Alpha: 2.0.0-alpha
    # Beta: 2.0.0-beta
    # RC: 2.0.0-rc1
    # Final: 2.0.0
    #
    # Since hyphens are a separator character between the upstream version
    # and Debian version, we replace these with tildes.
    #
    # Other goals:
    # - We would like to know the branch and commit of a package.
    # - We would like the PPA to trump the official Debian package.
    #
    # The following versions are sorted from low to high order:
    # 1.9.9
    # 2.0.0~alpha~pre
    # 2.0.0~alpha
    # 2.0.0~beta~pre
    # 2.0.0~beta
    # 2.0.0~dfsg4 <- official Debian package version
    # 2.0.0~rc1
    # 2.0.0
    # 2.0.1~alpha~pre
    #
    # Our official Debian packages have a ~dfsg section, so in this case an
    # rc1 package in our PPA would trump an official Debian package
    # (probably not what we want but not too bad since we would probably
    # publish a "2.0.0" final to our PPA before the official Debian package
    # is even released.
    #
    # Note in the above sorted list that if the branch name were included
    # after the mixxx_version, 2.0.0~master would sort earlier than
    # 2.0.0~rc1~master!  To prevent branch and revision tags from
    # interfering with package ordering we include them in the
    # debian_revision portion of the version. This ensures they are only
    # used for sorting if the upstream version of two packages is identical.
    upstream_version = mixxx_version.replace('-', '~')
    assert '_' not in upstream_version

    # Strip underscores and dashes in the branch name.
    branch_name = branch_name.strip('_-')
    assert branch_name and branch_name != '(no branch)'

    return "%s-%s~%s~%s%s~%s" % (upstream_version, ubuntu_version, branch_name,
                                 vcs_name, vcs_revision, distro_version)
