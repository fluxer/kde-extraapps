/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
 * Copyright (C) 2008 Harald Hvaal <haraldhv@stud.ntnu.no>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <KParts/MainWindow>
#include <KParts/ReadWritePart>
#include <KUrl>

class KRecentFilesAction;

class MainWindow: public KParts::MainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool loadPart();

    void dragEnterEvent(class QDragEnterEvent *event);
    void dropEvent(class QDropEvent *event);
    void dragMoveEvent(class QDragMoveEvent *event);

public slots:
    void openUrl(const KUrl& url);
    void setShowExtractDialog(bool);

private slots:
    void updateActions();
    void newArchive();
    void openArchive();
    void quit();

    void editKeyBindings();
    void editToolbars();

protected:
    // KMainWindow reimplementations
    void saveProperties(KConfigGroup &group) final;
    void readProperties(const KConfigGroup &group) final;

private:
    void setupActions();

    KParts::ReadWritePart *m_part;
    KRecentFilesAction    *m_recentFilesAction;
    QAction               *m_openAction;
    QAction               *m_newAction;
    KParts::OpenUrlArguments m_openArgs;
};

#endif // MAINWINDOW_H
