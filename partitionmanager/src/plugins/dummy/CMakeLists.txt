# Copyright (C) 2010 by Volker Lanz <vl@fidra.de>
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

file (GLOB pmdummybackendplugin_SRCS *.cpp)

kde4_add_plugin(pmdummybackendplugin ${pmdummybackendplugin_SRCS})

target_link_libraries(pmdummybackendplugin partitionmanagerprivate KDE4::kdecore KDE4::kio ${LIBPARTED_LIBS} ${BLKID_LIBRARIES} KDE4::solid)

install(TARGETS pmdummybackendplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES pmdummybackendplugin.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

