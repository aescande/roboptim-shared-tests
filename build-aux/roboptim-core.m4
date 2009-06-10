#                                                                -*-Autoconf-*-
# Copyright (C) 2009 by Thomas Moulard, FIXME.
# This file is part of the roboptim.
#
# roboptim is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Additional permission under section 7 of the GNU General Public
# License, version 3 ("GPLv3"):
#
# If you convey this file as part of a work that contains a
# configuration script generated by Autoconf, you may do so under
# terms of your choice.
#
# roboptim is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with roboptim.  If not, see <http://www.gnu.org/licenses/>.

# serial 2

# ------ #
# README #
# ------ #

# This m4 file can be used to check for the roboptim library.
# You can copy it into your software's aux dir and call the following
# macro to check for the library.
#
# ROBOPTIM_CORE_PKG_CONFIG: add a hard dependency against roboptim-core
# using pkg-config.

m4_pattern_forbid([^ROBOPTIM_CORE])

# ROBOPTIM_CORE_PKG_CONFIG([REQUIRED_VERSION])
# --------------------------------------------
# Search for roboptim-core using pkg-config.
# REQUIRED_VERSION represent how to check the version.
# Example: ``roboptim-core >= 0.1''
AC_DEFUN([ROBOPTIM_CORE_PKG_CONFIG],
[
PKG_CHECK_MODULES([ROBOPTIMCORE], m4_default($1, roboptim-core))

ROBOPTIMCORE_DOCDIR=`$PKG_CONFIG roboptim-core --variable=docdir`
AC_SUBST([ROBOPTIMCORE_DOCDIR])
AC_SUBST([ROBOPTIMCORE_CFLAGS])
AC_SUBST([ROBOPTIMCORE_LIBS])
]) # ROBOPTIM_CORE_PKG_CONFIG