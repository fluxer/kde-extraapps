/***********************************************************************
* Copyright 2003-2004  Max Howell <max.howell@methylblue.com>
* Copyright 2008-2009  Martin Sandsmark <martin.sandsmark@kde.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#ifndef FILELIGHTPART_H
#define FILELIGHTPART_H

#include <KParts/Part>
#include <KUrl>

#include <QtGui/QLabel>

namespace RadialMap {
class Widget;
}
class Folder;


namespace Filelight
{
class Part;

class Part : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    Part(QWidget *, QObject *, const QList<QVariant>&);

    virtual bool openFile() {
        return false;    //pure virtual in base class
    }
    virtual bool closeUrl();

    QString prettyUrl() const {
        return url().protocol() == QLatin1String( "file" ) ? url().path() : url().prettyUrl();
    }

public slots:
    virtual bool openUrl(const KUrl&);
    void configFilelight();
    void rescan();

private slots:
    void postInit();
    void scanCompleted(Folder*);
    void mapChanged(const Folder*);
    void clearStatusBarMessage();

private:
    void showSummary();

    QLayout            *m_layout;
    QWidget            *m_summary;
    RadialMap::Widget  *m_map;
    class ScanManager  *m_manager;

    bool m_started;

private:
    bool start(const KUrl&);

private slots:
    void updateURL(const KUrl &);
};
}

#endif
