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

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.22.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(CMAKE_USE_RELATIVE_PATHS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(BLKID REQUIRED blkid)
pkg_check_modules(UUID REQUIRED uuid)

find_package(LIBPARTED REQUIRED)
set_package_properties(LIBPARTED PROPERTIES
    DESCRIPTION "GNU Parted manipulates partition tables"
    URL "https://www.gnu.org/software/parted/"
    PURPOSE "Required for partitions management"
)

macro_optional_find_package(LibATASmart)
set_package_properties(LibATASmart PROPERTIES
    DESCRIPTION "ATA S.M.A.R.T. reading and parsing library"
    URL "http://0pointer.de/blog/projects/being-smart.html"
    PURPOSE "Required for ATA S.M.A.R.T. status support"
)

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

include_directories(
    ${UUID_INCLUDE_DIRS}
    ${BLKID_INCLUDE_DIRS}
    src/
)

add_subdirectory(src)
