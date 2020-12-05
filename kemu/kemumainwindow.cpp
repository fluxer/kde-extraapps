/*  This file is part of the KDE libraries
    Copyright (C) 2016 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <KAction>
#include <KActionCollection>
#include <KLocale>
#include <KIcon>
#include <KFileDialog>
#include <KInputDialog>
#include <KStandardDirs>
#include <KStatusBar>
#include <KDebug>
#include <ksettings.h>
#include <QApplication>
#include <QMessageBox>
#include <QThread>

#include "kemumainwindow.h"
#include "ui_kemu.h"

KEmuMainWindow::KEmuMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags), m_loading(false), m_installed(false), m_kemuui(new Ui_KEmuWindow)
{
    m_kemuui->setupUi(this);

    KAction *a = actionCollection()->addAction("harddisk_create", this, SLOT(createHardDisk()));
    a->setText(i18n("Create Hard Disk image"));
    a->setIcon(KIcon("drive-harddisk"));
    a->setShortcut(KStandardShortcut::openNew());
    a->setWhatsThis(i18n("Create a new Hard Disk image for later use."));

    KAction *b = actionCollection()->addAction("file_quit", this, SLOT(quit()));
    b->setText(i18n("Quit"));
    b->setIcon(KIcon("application-exit"));
    b->setShortcut(KStandardShortcut::quit());
    b->setWhatsThis(i18n("Close the application."));

    setupGUI();
    setAutoSaveSettings();

    setWindowIcon(KIcon("applications-engineering"));
    m_kemuui->groupBox->setEnabled(false);
    m_kemuui->startStopButton->setText(i18n("Start"));
    m_kemuui->startStopButton->setIcon(KIcon("system-run"));
    m_kemuui->CPUInput->setMaximum(QThread::idealThreadCount());
    statusBar()->showMessage(i18n("No machines running"));
    connect(m_kemuui->machinesList->listView()->selectionModel(),
        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
        this, SLOT(machineChanged(QItemSelection,QItemSelection)));
    connect(m_kemuui->machinesList, SIGNAL(added(QString)), this, SLOT(addMachine(QString)));
    connect(m_kemuui->startStopButton, SIGNAL(clicked()), this, SLOT(startStopMachine()));
    connect(m_kemuui->machinesList, SIGNAL(removed(QString)), this, SLOT(removeMachine(QString)));

    static const QStringList qemuBins = QStringList() << "qemu-system-aarch64"
        << "qemu-system-alpha"
        << "qemu-system-arm"
        << "qemu-system-cris"
        << "qemu-system-i386"
        << "qemu-system-lm32"
        << "qemu-system-m68k"
        << "qemu-system-microblaze"
        << "qemu-system-microblazeel"
        << "qemu-system-mips"
        << "qemu-system-mips64"
        << "qemu-system-mips64el"
        << "qemu-system-mipsel"
        << "qemu-system-moxie"
        << "qemu-system-or32"
        << "qemu-system-ppc"
        << "qemu-system-ppc64"
        << "qemu-system-ppcemb"
        << "qemu-system-s390x"
        << "qemu-system-sh4"
        << "qemu-system-sh4eb"
        << "qemu-system-sparc"
        << "qemu-system-sparc64"
        << "qemu-system-tricore"
        << "qemu-system-unicore32"
        << "qemu-system-x86_64"
        << "qemu-system-xtensa"
        << "qemu-system-xtensaeb";
    foreach (const QString bin, qemuBins) {
        if(!KStandardDirs::findExe(bin).isEmpty()) {
            m_installed = true;
            m_kemuui->systemComboBox->addItem(bin);
        }
    }

    m_settings = new KSettings("kemu", KSettings::SimpleConfig);
#ifndef QT_KATIE
    foreach(const QString machine, m_settings->childGroups()) {
#else
    QStringList addedmachines;
    foreach(const QString key, m_settings->keys()) {
        const int sepindex = key.indexOf("/");
        if (sepindex < 1) {
            continue;
        }
        QString machine = key.left(sepindex);
        if (addedmachines.contains(machine)) {
            continue;
        }
        addedmachines.append(machine);
#endif
        if (m_settings->value(machine + "/enable").toBool() == true) {
            m_kemuui->machinesList->insertItem(machine);
            machineLoad(machine);
        } else {
            kDebug() << "garbage machine" << machine;
        }
    }
    const QString lastSelected = m_settings->value("lastselected").toString();
    if (!lastSelected.isEmpty()) {
        m_kemuui->machinesList->listView()->keyboardSearch(lastSelected);
    }

    if(!m_installed) {
        m_kemuui->startStopButton->setEnabled(false);
        QMessageBox::critical(this, i18n("QEMU not available"), i18n("QEMU not available"));
        return;
    }

    QFileInfo kvmdev("/dev/kvm");
    if (!kvmdev.exists()) {
        const QString modprobeBin = KStandardDirs::findExe("modprobe");
        if (!modprobeBin.isEmpty()) {
            QProcess modprobe(this);
            modprobe.start(modprobeBin, QStringList() << "-b" << "kvm");
            modprobe.waitForFinished();
            if (!kvmdev.exists()) {
                QMessageBox::warning(this, i18n("KVM not available"), i18n("KVM not available"));
            }
        } else {
            kDebug() << "modprobe not found";
        }
    }

    connect(m_kemuui->CDROMInput, SIGNAL(textChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->HardDiskInput, SIGNAL(textChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->systemComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->videoComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->audioComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(machineSave(QString)));
    connect(m_kemuui->RAMInput, SIGNAL(valueChanged(int)), this, SLOT(machineSave(int)));
    connect(m_kemuui->CPUInput, SIGNAL(valueChanged(int)), this, SLOT(machineSave(int)));
    connect(m_kemuui->KVMCheckBox, SIGNAL(stateChanged(int)), this, SLOT(machineSave(int)));
    connect(m_kemuui->ACPICheckBox, SIGNAL(stateChanged(int)), this, SLOT(machineSave(int)));
    connect(m_kemuui->argumentsLineEdit, SIGNAL(textChanged(QString)), this, SLOT(machineSave(QString)));
}

KEmuMainWindow::~KEmuMainWindow()
{
    saveAutoSaveSettings();
    const QString lastSelected = m_kemuui->machinesList->currentText();
    if (!lastSelected.isEmpty()) {
        m_settings->setValue("lastselected", lastSelected);
    }
    m_settings->sync();
    m_settings->deleteLater();
    foreach(QProcess* machineProcess, m_machines) {
        const QString machine = m_machines.key(machineProcess);
        kDebug() << "stopping machine" << machine;
        machineProcess->terminate();
        machineProcess->deleteLater();
        m_machines.remove(machine);
    }
    delete m_kemuui;
}

void KEmuMainWindow::createHardDisk()
{
    const QString qemuImg = KStandardDirs::findExe("qemu-img");
    if (qemuImg.isEmpty()) {
        QMessageBox::warning(this, i18n("Requirements not met"),
            i18n("qemu-img not found"));
        return;
    }
    const QString diskPath = KFileDialog::getSaveFileName(KUrl(), QString(), this, i18n("Hard Disk path"));
    if (!diskPath.isEmpty()) {
        bool ok = false;
        const int diskSize = KInputDialog::getInteger(i18n("Hard Disk size"),
            i18n("Hard Disk size in MegaBytes"), 10, 10, INT_MAX, 1, 10, &ok, this);
        if (ok) {
            QProcess imageProcess(this);
            imageProcess.setProcessChannelMode(QProcess::MergedChannels);
            imageProcess.start(qemuImg, QStringList() << "create" << "-f" << "raw"
                << diskPath << QByteArray(QByteArray::number(diskSize) + QByteArray("M")));
            imageProcess.waitForFinished();
            if (imageProcess.exitCode() == 0) {
                QMessageBox::information(this, i18n("Success"),
                    i18n("Image successfully created"));
            } else {
                QMessageBox::warning(this, i18n("QEMU error"),
                    i18n("An error occured:\n%1", QString(imageProcess.readAll())));
            }
        }
    }
}

void KEmuMainWindow::quit()
{
    qApp->quit();
}

void KEmuMainWindow::updateStatus()
{
    if (m_machines.size() == 0) {
        statusBar()->showMessage(i18n("No machines running"));
    } else {
        QString machineNames;
        bool firstmachine = true;
        foreach (const QString name, m_machines.keys()) {
            if (firstmachine) {
                machineNames += name;
                firstmachine = false;
            } else {
                machineNames += ", " + name;
            }
        }
        const QString statusText = i18n("Machines running: %1 (%2)", machineNames, m_machines.size());
        statusBar()->showMessage(statusText);
    }
}

void KEmuMainWindow::machineSave(const QString ignored)
{
    Q_UNUSED(ignored);
    if (m_loading) {
        return;
    }
    const QString machine = m_kemuui->machinesList->currentText();
    kDebug() << "saving machine" << machine;
    m_settings->setValue(machine + "/cdrom", m_kemuui->CDROMInput->url().prettyUrl());
    m_settings->setValue(machine + "/harddisk", m_kemuui->HardDiskInput->url().prettyUrl());
    m_settings->setValue(machine + "/system", m_kemuui->systemComboBox->currentText());
    m_settings->setValue(machine + "/video", m_kemuui->videoComboBox->currentText());
    m_settings->setValue(machine + "/audio", m_kemuui->audioComboBox->currentText());
    m_settings->setValue(machine + "/ram", m_kemuui->RAMInput->value());
    m_settings->setValue(machine + "/cpu", m_kemuui->CPUInput->value());
    m_settings->setValue(machine + "/kvm", m_kemuui->KVMCheckBox->isChecked());
    m_settings->setValue(machine + "/acpi", m_kemuui->ACPICheckBox->isChecked());
    m_settings->setValue(machine + "/args", m_kemuui->argumentsLineEdit->text());
    m_settings->sync();
}

void KEmuMainWindow::machineSave(int ignored)
{
    Q_UNUSED(ignored);
    machineSave(QString());
}


void KEmuMainWindow::machineLoad(const QString machine)
{
    m_loading = true;
    kDebug() << "loading machine" << machine;
    m_kemuui->CDROMInput->setUrl(m_settings->value(machine + "/cdrom").toUrl());
    m_kemuui->HardDiskInput->setUrl(m_settings->value(machine + "/harddisk").toUrl());
    const QString system = m_settings->value(machine + "/system").toString();
    const int systemIndex = m_kemuui->systemComboBox->findText(system);
    m_kemuui->systemComboBox->setCurrentIndex(systemIndex);
    const QString video = m_settings->value(machine + "/video", "virtio").toString();
    const int videoIndex = m_kemuui->videoComboBox->findText(video);
    m_kemuui->videoComboBox->setCurrentIndex(videoIndex);
    const QString audio = m_settings->value(machine + "/audio", "ac97").toString();
    const int audioIndex = m_kemuui->audioComboBox->findText(audio);
    m_kemuui->audioComboBox->setCurrentIndex(audioIndex);
    m_kemuui->RAMInput->setValue(m_settings->value(machine + "/ram", 32).toInt());
    m_kemuui->CPUInput->setValue(m_settings->value(machine + "/cpu", 1).toInt());
    m_kemuui->KVMCheckBox->setChecked(m_settings->value(machine + "/kvm", false).toBool());
    m_kemuui->ACPICheckBox->setChecked(m_settings->value(machine + "/acpi", false).toBool());
    m_kemuui->argumentsLineEdit->setText(m_settings->value(machine + "/args").toString());
    m_loading = false;
}

void KEmuMainWindow::machineChanged(QItemSelection ignored, QItemSelection ignored2)
{
    Q_UNUSED(ignored);
    Q_UNUSED(ignored2);
    const QString machine = m_kemuui->machinesList->currentText();
    if (!machine.isEmpty()) {
        QFileInfo kvmdev("/dev/kvm");
        m_kemuui->KVMCheckBox->setEnabled(kvmdev.exists());

        m_kemuui->startStopButton->setEnabled(m_installed);
        m_kemuui->groupBox->setEnabled(true);
        if (m_machines.contains(machine)) {
            kDebug() << "machine is running" << machine;
            m_kemuui->startStopButton->setText(i18n("Stop"));
            m_kemuui->startStopButton->setIcon(KIcon("system-shutdown"));
        } else {
            kDebug() << "machine is stopped" << machine;
            m_kemuui->startStopButton->setText(i18n("Start"));
            m_kemuui->startStopButton->setIcon(KIcon("system-run"));
        }
        machineLoad(machine);
    } else {
        m_kemuui->startStopButton->setEnabled(false);
        m_kemuui->groupBox->setEnabled(false);
    }
}

void KEmuMainWindow::machineFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *machineProcess = qobject_cast<QProcess*>(sender());
    disconnect(machineProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
        this, SLOT(machineFinished(int,QProcess::ExitStatus)));
    if (exitCode != 0) {
        QMessageBox::warning(this, i18n("QEMU error"),
            i18n("An error occured:\n%1", QString(machineProcess->readAll())));
    }
    m_kemuui->startStopButton->setText(i18n("Start"));
    m_kemuui->startStopButton->setIcon(KIcon("system-run"));
    const QString machine = m_machines.key(machineProcess);
    m_machines.remove(machine);
    machineProcess->deleteLater();

    updateStatus();
}

void KEmuMainWindow::addMachine(const QString machine)
{
    m_settings->setValue(machine + "/enable", true);
    // TODO: set defaults?
    m_settings->sync();
}

void KEmuMainWindow::startStopMachine()
{
    const QString machine = m_kemuui->machinesList->currentText();
    if (!machine.isEmpty()) {
        if (m_machines.contains(machine)) {
            kDebug() << "stopping machine" << machine;
            QProcess* machineProcess = m_machines.take(machine);
            machineProcess->terminate();
            machineProcess->deleteLater();
            m_kemuui->startStopButton->setText(i18n("Start"));
            m_kemuui->startStopButton->setIcon(KIcon("system-run"));
        } else {
            kDebug() << "starting machine" << machine;
            QStringList machineArgs;
            machineArgs << "-name" << machine;
            const QString CDRom = m_kemuui->CDROMInput->url().prettyUrl();
            if (!CDRom.isEmpty()) {
                machineArgs << "-cdrom" << CDRom;
            }
            const QString HardDisk = m_kemuui->HardDiskInput->url().prettyUrl();
            if (!HardDisk.isEmpty()) {
                machineArgs << "-hda" << HardDisk;
            }
            if (CDRom.isEmpty() && HardDisk.isEmpty()) {
                QMessageBox::warning(this, i18n("Requirements not met"),
                    i18n("Either CD-ROM or Hard Disk image must be set"));
                return;
            }
            machineArgs << "-vga" << m_kemuui->videoComboBox->currentText();
            machineArgs << "-soundhw" << m_kemuui->audioComboBox->currentText();
            machineArgs << "-m" << QByteArray::number(m_kemuui->RAMInput->value());
            machineArgs << "-smp" << QByteArray::number(m_kemuui->CPUInput->value());
            if (m_kemuui->KVMCheckBox->isEnabled() && m_kemuui->KVMCheckBox->isChecked()) {
                machineArgs << "-enable-kvm";
            }
            if (!m_kemuui->ACPICheckBox->isChecked()) {
                machineArgs << "-no-acpi";
            }
            const QString extraArgs = m_kemuui->argumentsLineEdit->text();
            if (!extraArgs.isEmpty()) {
                foreach (const QString argument, extraArgs.split(" ")) {
                    machineArgs << argument;
                }
            }
            QProcess* machineProcess = new QProcess(this);
            machineProcess->setProcessChannelMode(QProcess::MergedChannels);
            m_kemuui->startStopButton->setText(i18n("Stop"));
            m_kemuui->startStopButton->setIcon(KIcon("system-shutdown"));
            m_machines.insert(machine, machineProcess);
            connect(machineProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(machineFinished(int,QProcess::ExitStatus)));
            machineProcess->start(m_kemuui->systemComboBox->currentText(), machineArgs);
        }

        updateStatus();
    }
}

void KEmuMainWindow::removeMachine(const QString machine)
{
    if (m_machines.contains(machine)) {
        kDebug() << "stopping machine" << machine;
        QProcess* machineProcess = m_machines.take(machine);
        machineProcess->terminate();
        machineProcess->deleteLater();
    }
    kDebug() << "removing machine" << machine;
    m_settings->setValue(machine + "/enable", false);
    m_settings->sync();
}
