/*************************************************************************************
 *  Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *  Copyright (C) 2012, 2013 by Daniel Vrátil <dvratil@redhat.com>                   *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Lesser General Public                       *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2.1 of the License, or (at your option) any later version.               *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Lesser General Public License for more details.                                  *
 *                                                                                   *
 *  You should have received a copy of the GNU Lesser General Public                 *
 *  License along with this library; if not, write to the Free Software              *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA       *
 *************************************************************************************/

#include "xrandrconfig.h"
#include "xrandrscreen.h"
#include "xrandr.h"
#include "xrandrmode.h"
#include "xrandroutput.h"
#include "config.h"
#include "output.h"
#include "edid.h"

#include <QX11Info>
#include <QRect>

#include <kdebug.h>

#include "libkscreen-config.h"

using namespace KScreen;

XRandRConfig::XRandRConfig()
    : QObject()
    , m_primaryOutput(-1)
    , m_screen(new XRandRScreen(this))
{
    XRRScreenResources* resources = XRandR::screenResources();

    RROutput id, primary;
    primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());

    XRandROutput::Map outputs;
    for (int i = 0; i < resources->noutput; ++i)
    {
        id = resources->outputs[i];

        XRandROutput *output = createNewOutput(id, (id == primary));
        m_outputs.insert(id, output);
        if (id == primary) {
            m_primaryOutput = output->id();
        }
    }

    XRRFreeScreenResources(resources);
}

XRandRConfig::~XRandRConfig()
{
}

void XRandRConfig::update()
{
    m_screen->update();

    RROutput primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());

    m_primaryOutput = -1;
    XRandROutput::Map::Iterator iter;
    for (iter = m_outputs.begin(); iter != m_outputs.end(); ++iter) {
        XRandROutput *output = iter.value();
        output->update((iter.key() == (int) primary) ? XRandROutput::SetPrimary : XRandROutput::UnsetPrimary);
        if (iter.key() == (int) primary) {
            m_primaryOutput = output->id();
        }
    }
}

XRandROutput::Map XRandRConfig::outputs() const
{
    return m_outputs;
}

void XRandRConfig::addNewOutput(const RROutput id)
{
    RROutput primary;
    primary = XRRGetOutputPrimary(XRandR::display(), XRandR::rootWindow());
    XRandROutput *output = createNewOutput(id, (id == primary));
    m_outputs.insert(id, output);
    if (id == primary) {
        m_primaryOutput = id;
    }
}

XRandROutput* XRandRConfig::createNewOutput(RROutput id, bool primary)
{
    XRandROutput *xOutput = new XRandROutput(id, primary, this);
    connect(xOutput, SIGNAL(outputRemoved(int)), SLOT(outputRemovedSlot(int)));

    return xOutput;
}

void XRandRConfig::outputRemovedSlot(int id)
{
    m_outputs.remove(id);
    Q_EMIT outputRemoved(id);
}

KScreen::Config *XRandRConfig::toKScreenConfig() const
{
    KScreen::Config *config = new KScreen::Config();
    KScreen::OutputList kscreenOutputs;

    XRandROutput::Map::ConstIterator iter;
    for (iter = m_outputs.constBegin(); iter != m_outputs.constEnd(); ++iter) {
        XRandROutput *output = iter.value();
        //FIXME XRandR backend should keep updated itself
        output->update(XRandROutput::NoChange);
        KScreen::Output *kscreenOutput = output->toKScreenOutput(config);
        kscreenOutputs.insert(kscreenOutput->id(), kscreenOutput);
    }

    config->setOutputs(kscreenOutputs);
    config->setScreen(m_screen->toKScreenScreen(config));
    if (m_primaryOutput != -1 && (!config->primaryOutput() || config->primaryOutput()->id() != m_primaryOutput)) {
        config->setPrimaryOutput(kscreenOutputs.value(m_primaryOutput));
    }

    return config;
}

void XRandRConfig::updateKScreenConfig(Config *config) const
{
    KScreen::Screen *kscreenScreen = config->screen();
    m_screen->updateKScreenScreen(kscreenScreen);

    //Removing removed outputs
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH(KScreen::Output *output, outputs) {
        if (!m_outputs.contains(output->id())) {
            config->removeOutput(output->id());
        }
    }

    XRandROutput::Map::ConstIterator iter;
    for (iter = m_outputs.constBegin(); iter != m_outputs.constEnd(); ++iter) {
        XRandROutput *output = iter.value();
        KScreen::Output *kscreenOutput = config->output(output->id());

        if (!kscreenOutput) {
            config->addOutput(output->toKScreenOutput(config));
            continue;
        }
        output->updateKScreenOutput(kscreenOutput);
    }

    if (!config->primaryOutput() || config->primaryOutput()->id() != m_primaryOutput) {
        config->setPrimaryOutput(config->output(m_primaryOutput));
    }
}

void XRandRConfig::applyKScreenConfig(KScreen::Config *config)
{
    KDebug::Block apply("Applying KScreen Config", dXndr());
    KScreen::OutputList outputs = config->outputs();
    QSize newSize = screenSize(config);

    int neededCrtc = 0;
    int primaryOutput = 0;
    KScreen::OutputList toDisable, toEnable, toChange;
    QHash<int, int> currentCrtc;
    Q_FOREACH(KScreen::Output *output, outputs) {
        XRandROutput *currentOutput = m_outputs.value(output->id());
        currentOutput->update(currentOutput->isPrimary() ? XRandROutput::SetPrimary : XRandROutput::UnsetPrimary);

        //Only set the output as primary if it is enabled.
        if (output->isPrimary() && output->isEnabled()) {
            primaryOutput = currentOutput->id();
        }

        bool currentEnabled = currentOutput->isEnabled();
        if (!output->isEnabled() && currentEnabled) {
            toDisable.insert(output->id(), output);
            continue;
        } else if (output->isEnabled() && !currentEnabled) {
            toEnable.insert(output->id(), output);
            neededCrtc ++;
            continue;
        } else if (!output->isEnabled() && !currentEnabled) {
            continue;
        }

        neededCrtc ++;

        if (output->currentModeId() != currentOutput->currentModeId()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), XRandR::outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }

        if (output->pos() != currentOutput->position()) {
            if (!toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), XRandR::outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }

        if (output->rotation() != currentOutput->rotation()) {
            if( !toChange.contains(output->id())) {
                currentCrtc.insert(output->id(), XRandR::outputCrtc(output->id()));
                toChange.insert(output->id(), output);
            }
        }

        XRandRMode* currentMode = currentOutput->currentMode();

        // For some reason, in some environments currentMode is null
        // which doesn't make sense because it is the *current* mode...
        // Since we haven't been able to figure out the reason why
        // this happens, we are adding this debug code to try to
        // figure out how this happened.
        if (!currentMode) {
            kWarning() << "Current mode is null:"
            << "ModeId:" << currentOutput->currentModeId()
            << "Mode: " << currentOutput->currentMode()
            << "Output: " << currentOutput->id();
            kDebug() << kBacktrace(256);
            printConfig(config);
            printInternalCond();
            return;
        }

        const QSize size = currentMode->size();
        int x, y;

        //TODO: Move this code within libkscreen
        y = currentOutput->position().y();
        if (currentOutput->isHorizontal()) {
            y += size.height();
        } else {
            y += size.width();
        }

        x = currentOutput->position().x();
        if (currentOutput->isHorizontal()) {
            x += size.width();
        } else {
            x += size.height();
        }

        if (x > newSize.width() || y > newSize.height()) {
            if (!toDisable.contains(output->id())) {
                kDebug(dXndr()) << "Output doesn't fit: " << x << "x" << y << newSize;
                toDisable.insert(output->id(), output);
            }
        }
    }//Q_FOREACH(KScreen::Output *output, outputs)

    KScreen::Screen* screen = config->screen();
    if (newSize.width() > screen->maxSize().width() ||
        newSize.height() > screen->maxSize().height()) {
        kDebug(dXndr()) << "The new size is too big: " << newSize << " - " << screen->maxSize();
        return;//Too big
    }

    kDebug(dXndr()) << neededCrtc;
    XRRScreenResources *screenResources = XRandR::screenResources();
    if (neededCrtc > screenResources->ncrtc) {
        kDebug(dXndr()) << "We need more crtc than we have: " << neededCrtc << " - " << screenResources->ncrtc;
        XRRFreeScreenResources(screenResources);
        return;//We don't have enough crtc
    }
    XRRFreeScreenResources(screenResources);

    kDebug(dXndr()) << "Actions to perform: ";
    kDebug(dXndr()) << "\t Primary Output: " << primaryOutput;
    kDebug(dXndr()) << "\t Screen Size: " << (newSize != m_screen->currentSize());
    if (newSize != m_screen->currentSize()) {
        kDebug(dXndr()) << "\t Old: " << m_screen->currentSize();
        kDebug(dXndr()) << "\t New: " << newSize;
    }
    kDebug(dXndr()) << "\t Disable outputs: " << !toDisable.isEmpty();
    if (!toDisable.isEmpty()) {
        kDebug(dXndr()) << "\t\t" << toDisable.keys();
    }
    kDebug(dXndr()) << "\t Change outputs: " << !toChange.isEmpty();
    if (!toChange.isEmpty()) {
        kDebug(dXndr()) << "\t\t" << toChange.keys();
    }
    kDebug(dXndr()) << "\t Enable outputs: " << !toEnable.isEmpty();
    if (!toEnable.isEmpty()) {
        kDebug(dXndr()) << "\t\t" << toEnable.keys();
    }

    setPrimaryOutput(primaryOutput);

    //If there is nothing to do, not even bother
    if (toDisable.isEmpty() && toEnable.isEmpty() && toChange.isEmpty()) {
        if (newSize != m_screen->currentSize()) {
            setScreenSize(newSize);
        }
        return;
    }

    Q_FOREACH(KScreen::Output* output, toDisable) {
        disableOutput(output);
    }

    if (newSize != m_screen->currentSize()) {
        setScreenSize(newSize);
    }

    bool forceScreenSizeUpdate = false;
    Q_FOREACH(KScreen::Output* output, toChange) {
        if (!changeOutput(output, currentCrtc[output->id()])) {

            /* If we disabled the output before changing it and XRandR failed
             * to re-enable it, then update screen size too */
            if (toDisable.contains(output->id())) {
                output->setEnabled(false);
                kDebug() << "Output failed to change: " << output->name();
                forceScreenSizeUpdate = true;
            }
        }
    }

    Q_FOREACH(KScreen::Output* output, toEnable) {
        if (!enableOutput(output)) {
            output->setEnabled(false);
            kDebug() << "Output failed to be Enabled: " << output->name();
            forceScreenSizeUpdate = true;
        }
    }


    if (forceScreenSizeUpdate) {
        newSize = screenSize(config);
        kDebug() << "forced to change screen Size: " << newSize;
        setScreenSize(newSize);
    }
}

void XRandRConfig::printConfig(Config* config) const
{
    kDebug() << "KScreen version:" << LIBKSCREEN_VERSION;

    if (!config) {
        kDebug() << "Config is invalid";
        return;
    }
    if (!config->screen()) {
        kDebug() << "No screen in the configuration, broken backend";
        return;
    }

    kDebug() << "Screen:";
    kDebug() << "\tmaxSize:" << config->screen()->maxSize();
    kDebug() << "\tminSize:" << config->screen()->minSize();
    kDebug() << "\tcurrentSize:" << config->screen()->currentSize();

    OutputList outputs = config->outputs();
    Q_FOREACH(Output *output, outputs) {
        kDebug() << "\n-----------------------------------------------------\n";
        kDebug() << "Id: " << output->id();
        kDebug() << "Name: " << output->name();
        kDebug() << "Type: " << output->type();
        kDebug() << "Connected: " << output->isConnected();
        if (!output->isConnected()) {
            continue;
        }
        kDebug() << "Enabled: " << output->isEnabled();
        kDebug() << "Primary: " << output->isPrimary();
        kDebug() << "Rotation: " << output->rotation();
        kDebug() << "Pos: " << output->pos();
        kDebug() << "MMSize: " << output->sizeMm();
        if (output->currentMode()) {
            kDebug() << "Size: " << output->currentMode()->size();
        }
        if (output->clones().isEmpty()) {
            kDebug() << "Clones: " << "None";
        } else {
            kDebug() << "Clones: " << output->clones().count();
        }
        kDebug() << "Mode: " << output->currentModeId();
        kDebug() << "Preferred Mode: " << output->preferredModeId();
        kDebug() << "Preferred modes: " << output->preferredModes();
        kDebug() << "Modes: ";

        ModeList modes = output->modes();
        Q_FOREACH(Mode* mode, modes) {
            kDebug() << "\t" << mode->id() << "  " << mode->name() << " " << mode->size() << " " << mode->refreshRate();
        }

        Edid* edid = output->edid();
        kDebug() << "EDID Info: ";
        if (edid && edid->isValid()) {
            kDebug() << "\tDevice ID: " << edid->deviceId();
            kDebug() << "\tName: " << edid->name();
            kDebug() << "\tVendor: " << edid->vendor();
            kDebug() << "\tSerial: " << edid->serial();
            kDebug() << "\tEISA ID: " << edid->eisaId();
            kDebug() << "\tHash: " << edid->hash();
            kDebug() << "\tWidth: " << edid->width();
            kDebug() << "\tHeight: " << edid->height();
            kDebug() << "\tGamma: " << edid->gamma();
            kDebug() << "\tRed: " << edid->red();
            kDebug() << "\tGreen: " << edid->green();
            kDebug() << "\tBlue: " << edid->blue();
            kDebug() << "\tWhite: " << edid->white();
        } else {
            kDebug() << "\tUnavailable";
        }
    }
}

void XRandRConfig::printInternalCond() const
{
    kDebug() << "Internal config in xrandr";
    Q_FOREACH(XRandROutput *output, m_outputs) {
        kDebug() << "Id: " << output->id();
        kDebug() << "Current Mode: " << output->currentMode();
        kDebug() << "Current mode id: " << output->currentModeId();
        kDebug() << "Connected: " << output->isConnected();
        kDebug() << "Enabled: " << output->isEnabled();
        kDebug() << "Primary: " << output->isPrimary();
        if (!output->isEnabled()) {
            continue;
        }
        XRandRMode::Map modes = output->modes();
        Q_FOREACH(XRandRMode *mode, modes) {
            kDebug() << "\t" << mode->id();
            kDebug() << "\t" << mode->name();
            kDebug() << "\t" << mode->size() << mode->refreshRate();
        }
    }
}

QSize XRandRConfig::screenSize(KScreen::Config* config) const
{
    KDebug::Block screenBlock("Calculating screen size", dXndr());
    QRect rect;
    QSize outputSize;
    Q_FOREACH(const KScreen::Output* output, config->outputs()) {
        if (!output->isEnabled() || !output->isConnected()) {
            kDebug(dXndr()) << "Disabled/Disconnected output: " << output->name();
            continue;
        }

        Mode *currentMode = output->currentMode();
        if (!currentMode) {
            kDebug(dXndr()) << "Output: " << output->name() << " has no current Mode";
            continue;
        }

        QSize outputSize = currentMode->size();

        kDebug(dXndr()) << "Output: " << output->name() << " Size: " << outputSize << " Pos: " << output->pos();
        if (output->pos().x() < rect.x()) {
            rect.setX(output->pos().x());
        }

        if (output->pos().y() < rect.y()) {
            rect.setY(output->pos().y());
        }

        QPoint bottomRight;
        if (output->isHorizontal()) {
            bottomRight = QPoint(output->pos().x() + outputSize.width(),
                                 output->pos().y() + outputSize.height());
        } else {
            bottomRight = QPoint(output->pos().x() + outputSize.height(),
                                 output->pos().y() + outputSize.width());
        }

        if (bottomRight.x() > rect.width()) {
            rect.setWidth(bottomRight.x());
        }

        if (bottomRight.y() > rect.height()) {
            rect.setHeight(bottomRight.y());
        }

    }

    QSize size = QSize(rect.width(), rect.height());
    kDebug(dXndr()) << "Requested screen size is" << size;
    return size;
}

bool XRandRConfig::setScreenSize(const QSize& size) const
{
    KDebug::Block setBlock("Setting screen size", dXndr());
    double dpi;
    int widthMM, heightMM;
    dpi = (25.4 * DisplayHeight(XRandR::display(), XRandR::screen())) / DisplayHeightMM(XRandR::display(), XRandR::screen());

    kDebug(dXndr()) << "DPI: " << dpi;
    kDebug(dXndr()) << "Size: " << size;

    widthMM =  ((25.4 * size.width()) / dpi);
    heightMM = ((25.4 * size.height()) / dpi);

    kDebug(dXndr()) << size << " " << widthMM << "x" << heightMM;
    XRRSetScreenSize(XRandR::display(), XRandR::rootWindow(),
                     size.width(), size.height(), widthMM, heightMM);

    return true;
}

void XRandRConfig::setPrimaryOutput(int outputId) const
{
    XRRSetOutputPrimary(XRandR::display(), XRandR::rootWindow(), outputId);
}

bool XRandRConfig::disableOutput(Output* output) const
{
    KDebug::Block disablock("Disable output", dXndr());
    int crtcId = XRandR::outputCrtc(output->id());
    kDebug(dXndr()) << "Disabling: " << output->id() << "(CRTC" << crtcId << ")";
    Status s = XRRSetCrtcConfig (XRandR::display(), XRandR::screenResources(), crtcId, CurrentTime,
                 0, 0, None, RR_Rotate_0, NULL, 0);

    kDebug(dXndr()) << "XRRSetCrtcConfig() returned" << s;

    // Update the cached output now, otherwise we get RRNotify_CrtcChange notification
    // for an outdated output, which can lead to a crash.
    if (s == RRSetConfigSuccess) {
        m_outputs.value(output->id())->update();
    }
    return (s == RRSetConfigSuccess);
}

bool XRandRConfig::enableOutput(Output* output) const
{
    KDebug::Block disablock("Enable output", dXndr());
    kDebug(dXndr()) << "Enabling: " << output->id();
    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(XRandR::display(), XRandR::screenResources(), XRandR::freeCrtc(output->id()),
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentModeId().toInt(),
        output->rotation(), outputs, 1);

    kDebug(dXndr()) << "XRRSetCrtcConfig() returned" << s;
    return (s == RRSetConfigSuccess);
}

bool XRandRConfig::changeOutput(Output* output, int crtcId) const
{
    KDebug::Block disablock("Change output", dXndr());
    kDebug(dXndr()) << "Updating: " << output->id() << "with CRTC" << crtcId;

    RROutput *outputs = new RROutput[1];
    outputs[0] = output->id();
    Status s = XRRSetCrtcConfig(XRandR::display(), XRandR::screenResources(), crtcId,
        CurrentTime, output->pos().rx(), output->pos().ry(), output->currentModeId().toInt(),
        output->rotation(), outputs, 1);

    kDebug(dXndr()) << "XRRSetCrtcConfig() returned" << s;
    return (s == RRSetConfigSuccess);
}

#include "moc_xrandrconfig.cpp"
