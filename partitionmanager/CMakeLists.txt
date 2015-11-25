# Copyright (C) 2008 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

project(partitionmanager)

set(CMAKE_USE_RELATIVE_PATHS OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(VERSION_MAJOR "1")
set(VERSION_MINOR "1")
set(VERSION_RELEASE "1")
set(VERSION_SUFFIX "")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}${VERSION_SUFFIX})
add_definitions(-DVERSION="${VERSION}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLKID REQUIRED blkid)
pkg_check_modules(LIBATASMART REQUIRED libatasmart)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

include_directories(${UUID_INCLUDE_DIRS} ${BLKID_INCLUDE_DIRS} lib/ src/)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(icons)
