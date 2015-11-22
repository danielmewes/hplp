#! /bin/sh

# Maintainer script for automating the cross-compilation and installation of hplp
# from a checkout of the complete hplp repository over at Github.
# Simplified from the tilp / gfm script (recompile_cross_tilp.sh).
#
# Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015 Lionel Debroux, Benjamin Moody


# REMINDERS: don't forget to:
# 1) create AND chmod +x a "i686-w64-mingw32-pkg-config" script in the path:
# #!/bin/sh
# export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/lib
# exec pkg-config $@
# (this is for my Debian Wheezy/sid, at least)
#
# 2) have Wine registered through binfmt-support if it wasn't already done:
# update-binfmts --install wine <path_to_wine> --magic MZ

# The prefix where the binaries will be installed, e.g. $HOME, /usr, /usr/local.
PREFIX="$HOME/lpg/packages"

# Common flags and definitions.
CCFLAGS="" # This set of flags is intentionally small, the bulk is set by configure.ac.
CCPPFLAGS="-I/$HOME/lpg/deps/gtk-win32/include -I$HOME/lpg/depsprefix/include -D_FORTIFY_SOURCE=2" # The gtk dev files contains a libintl for Windows
CLDFLAGS="-L$HOME/lpg/deps/gtk-win32/lib -L$HOME/lpg/depsprefix/lib -static-libgcc" # The gtk dev files contains a libintl for Windows
CHOST="i686-w64-mingw32"

export PKG_CONFIG_PATH=$HOME/lpg/packages/lib/pkgconfig:$HOME/lpg/depsprefix/lib/pkgconfig:$PKG_CONFIG_PATH
export PKG_CONFIG_LIBDIR=$HOME/lpg/pkgconfig

# Configure and build the given module
handle_one_module() {
  module_name="$1"
  shift # Kick the first argument, so as to be able to pass the rest to configure.

  cd "$module_name"
  echo "Configuring $module_name"
  rm -f config.cache
  ./configure --host="$CHOST" CPPFLAGS="$CCPPFLAGS" CFLAGS="$CCFLAGS $CFLAGS" CXXFLAGS="$CCFLAGS $CFLAGS" LDFLAGS="$CLDFLAGS" --prefix="$PREFIX" $@ || return 1
  echo "Building $module_name"
  make clean || return 1
  make || return 1
  echo "Installing $module_name"
  make check || return 1
  make install || return 1
  cd -
}


if [ "x$NOAUTORECONF" = "x" -a "x$NO_AUTORECONF" = "x" ]; then
echo "=== AUTORECONF ==="
sh run_autoreconf.sh
fi

echo "=== UPDATEPOT ==="
sh run_updatepot.sh


sed "s/+ _nl_msg.*$//" -i libhpcalcs/configure


echo "=== libhpcalcs ==="
handle_one_module libhpcalcs || exit 1
