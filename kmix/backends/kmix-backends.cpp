/*
 *              KMix -- KDE's full featured mini mixer
 *
 *
 *              Copyright (C) 1996-2000 Christian Esken
 *                        esken@kde.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/* This code is being #include'd from mixer.cpp */

#include "mixer_backend.h"
#include "core/mixer.h"

#include <QString>


#if defined(Q_OS_SOLARIS)
#define SUN_MIXER
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD) || defined(Q_OS_DRAGONFLY) || defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)
#define OSS_MIXER
#endif

// PORTING: add #ifdef PLATFORM , commands , #endif, add your new mixer below

// Compiled by its own!
//#include "backends/mixer_mpris2.cpp"

#if defined(SUN_MIXER)
#include "backends/mixer_sun.cpp"
#endif

// OSS 3 / 4
#if defined(OSS_MIXER)
#include "backends/mixer_oss.cpp"

#if !defined(Q_OS_NETBSD) && !defined(Q_OS_OPENBSD)
#include <sys/soundcard.h>
#else
#include <soundcard.h>
#endif
#if !defined(Q_OS_FREEBSD) && (SOUND_VERSION >= 0x040000)
#define OSS4_MIXER
#endif
#endif

#if defined(OSS4_MIXER)
#include "backends/mixer_oss4.cpp"
#endif


// Possibly encapsualte by #ifdef HAVE_DBUS
Mixer_Backend* MPRIS2_getMixer(Mixer *mixer, int device );
QString MPRIS2_getDriverName();

Mixer_Backend* ALSA_getMixer(Mixer *mixer, int device );
QString ALSA_getDriverName();

MixerFactory g_mixerFactories[] = {

#if defined(SUN_MIXER)
    { SUN_getMixer, SUN_getDriverName },
#endif

#if defined(HAVE_ALSA)
    { ALSA_getMixer, ALSA_getDriverName },
#endif

#if defined(OSS_MIXER)
    { OSS_getMixer, OSS_getDriverName },
#endif

#if defined(OSS4_MIXER)
    { OSS4_getMixer, OSS4_getDriverName },
#endif

    // Make sure MPRIS2 is at the end. Implementation of SINGLE_PLUS_MPRIS2 in MixerToolBox is much easier.
    // And also we make sure, streams are always the last backend, which is important for the default KMix GUI layout.
    { MPRIS2_getMixer, MPRIS2_getDriverName },

    { 0, 0 }
};

