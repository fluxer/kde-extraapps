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

#ifndef KEMUMAINWINDOW_H
#define KEMUMAINWINDOW_H

#include <KXmlGuiWindow>
#include <QListWidgetItem>
#include <QSettings>
#include <QProcess>

QT_BEGIN_NAMESPACE
class Ui_KEmuWindow;
QT_END_NAMESPACE

class KEmuMainWindow: public KXmlGuiWindow
{
    Q_OBJECT
public:
    KEmuMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~KEmuMainWindow();

public slots:
    void createHardDisk();
    void quit();

private slots:
    void machineLoad(const QString machine);
    void machineSave(const QString ignored);
    void machineSave(int ignored);
    void machineChanged(QItemSelection ignored, QItemSelection ignored2);
    void machineFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void addMachine(const QString machine);
    void startStopMachine();
    void removeMachine(const QString machine);

private:
    void updateStatus();

    bool m_loading;
    bool m_installed;
    Ui_KEmuWindow *m_kemuui;
    QSettings *m_settings;
    QHash<QString,QProcess*> m_machines;
};

#endif // KEMUMAINWINDOW_H