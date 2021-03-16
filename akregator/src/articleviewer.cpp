/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "articleviewer.h"
#include "akregatorconfig.h"
#include "aboutdata.h"
#include "actionmanager.h"
#include "actions.h"
#include "article.h"
#include "articleformatter.h"
#include "articlejobs.h"
#include "articlematcher.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "utils.h"
#include "openurlrequest.h"

#include <kdeversion.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kglobalsettings.h>
#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kparts/browserrun.h>
#include <kmimetypetrader.h>

#include <QClipboard>
#include <QGridLayout>
#include <QWidget>

#include <boost/bind.hpp>

#include <memory>
#include <cassert>

using namespace boost;
using namespace Akregator;
using namespace Akregator::Filters;

namespace Akregator {

ArticleViewer::ArticleViewer(QWidget *parent)
    : QWidget(parent),
      m_url(0),
      m_htmlFooter(),
      m_currentText(),
      m_imageDir( KUrl::fromPath( KGlobal::dirs()->saveLocation("cache", "akregator/Media/" ) ) ),
      m_node(0),
      m_viewMode(NormalView),
      m_part( new ArticleViewerPart( this ) )
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_part->widget(), 0, 0);

    setFocusProxy( m_part->widget() );

    // change the cursor when loading stuff...
    connect( m_part, SIGNAL(started(KIO::Job*)),
             this, SLOT(slotStarted(KIO::Job*)));
    connect( m_part, SIGNAL(completed()),
             this, SLOT(slotCompleted()));

    KParts::BrowserExtension* ext = m_part->browserExtension();
    connect(ext, SIGNAL(popupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags,KParts::BrowserExtension::ActionGroupMap)),
             this, SLOT(slotPopupMenu(QPoint,KUrl,mode_t,KParts::OpenUrlArguments,KParts::BrowserArguments,KParts::BrowserExtension::PopupFlags))); // ActionGroupMap argument removed, unused by slot

    connect( ext, SIGNAL(openUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)),
             this, SLOT(slotOpenUrlRequestDelayed(KUrl,KParts::OpenUrlArguments,KParts::BrowserArguments)) );

    connect(ext, SIGNAL(createNewWindow(KUrl,
            KParts::OpenUrlArguments,
            KParts::BrowserArguments,
            KParts::WindowArgs,
            KParts::ReadOnlyPart**)),
            this, SLOT(slotCreateNewWindow(KUrl,
                         KParts::OpenUrlArguments,
                         KParts::BrowserArguments,
                         KParts::WindowArgs,
                         KParts::ReadOnlyPart**)));

    KAction* action = 0;

    action = m_part->actionCollection()->addAction("copylinkaddress");
    action->setText(i18n("Copy &Link Address"));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotCopyLinkAddress()));

    action = m_part->actionCollection()->addAction("savelinkas");
    action->setText(i18n("&Save Link As..."));
    connect(action, SIGNAL(triggered(bool)), SLOT(slotSaveLinkAs()));

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(slotPaletteOrFontChanged()) );
    connect(KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), this, SLOT(slotPaletteOrFontChanged()) );

    m_htmlFooter = "</body></html>";
}

ArticleViewer::~ArticleViewer()
{
}

KParts::ReadOnlyPart* ArticleViewer::part() const
{
    return m_part;
}

void ArticleViewer::slotOpenUrlRequestDelayed(const KUrl& url, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browserArgs)
{
    OpenUrlRequest req(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    if (req.options() == OpenUrlRequest::None)                // no explicit new window,
        req.setOptions(OpenUrlRequest::NewTab);               // so must open new tab

    if (m_part->button() == Qt::LeftButton)
    {
        switch (Settings::lMBBehaviour())
        {
            case Settings::EnumLMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }
    else if (m_part->button() == Qt::MiddleButton)
    {
        switch (Settings::mMBBehaviour())
        {
            case Settings::EnumMMBBehaviour::OpenInBackground:
                req.setOpenInBackground(true);
                break;
            default:
                break;
        }
    }

    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotCreateNewWindow(const KUrl& url,
                                       const KParts::OpenUrlArguments& args,
                                       const KParts::BrowserArguments& browserArgs,
                                       const KParts::WindowArgs& /*windowArgs*/,
                                       KParts::ReadOnlyPart** part)
{
    OpenUrlRequest req;
    req.setUrl(url);
    req.setArgs(args);
    req.setBrowserArgs(browserArgs);
    req.setOptions(OpenUrlRequest::NewTab);

    emit signalOpenUrlRequest(req);
    if ( part )
        *part = req.part();
}

void ArticleViewer::slotPopupMenu(const QPoint& p, const KUrl& kurl, mode_t, const KParts::OpenUrlArguments&, const KParts::BrowserArguments&, KParts::BrowserExtension::PopupFlags kpf)
{
    const bool isLink = (kpf & KParts::BrowserExtension::ShowNavigationItems) == 0; // ## why not use kpf & IsLink ?
    const bool isSelection = (kpf & KParts::BrowserExtension::ShowTextSelectionItems) != 0;

    QString url = kurl.url();

    m_url = url;
    KMenu popup;

    if (isLink && !isSelection)
    {
        popup.addAction( createOpenLinkInNewTabAction( kurl, this, SLOT(slotOpenLinkInForegroundTab()), &popup ) );
        popup.addAction( createOpenLinkInExternalBrowserAction( kurl, this, SLOT(slotOpenLinkInBrowser()), &popup ) );
        popup.addSeparator();
        popup.addAction( m_part->action("savelinkas") );
        popup.addAction( m_part->action("copylinkaddress") );
    }
    else
    {
        if (isSelection)
        {
            popup.addAction( ActionManager::getInstance()->action("viewer_copy") );
            popup.addSeparator();
        }
        popup.addAction( ActionManager::getInstance()->action("viewer_print") );
       //KAction *ac = action("setEncoding");
       //if (ac)
       //     ac->plug(&popup);
        popup.addSeparator();
        popup.addAction( ActionManager::getInstance()->action("inc_font_sizes") );
        popup.addAction( ActionManager::getInstance()->action("dec_font_sizes") );
    }
    popup.exec(p);
}

void ArticleViewer::slotCopyLinkAddress()
{
    if(m_url.isEmpty()) return;
    QClipboard *cb = QApplication::clipboard();
    cb->setText(m_url.prettyUrl(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    // which spec? Nobody seems to care (tested Firefox (3.5.10) Konqueror,and KMail (4.2.3)), so I re-enable the following line unless someone gives
    // a good reason to remove it again (bug 183022) --Frank
    cb->setText(m_url.prettyUrl(), QClipboard::Selection);
}

void ArticleViewer::slotOpenLinkInternal()
{
    openUrl(m_url);
}

void ArticleViewer::slotOpenLinkInForegroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(m_url);
    req.setOptions(OpenUrlRequest::NewTab);
    req.setOpenInBackground(true);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(m_url);
    emit signalOpenUrlRequest(req);
}

void ArticleViewer::slotSaveLinkAs()
{
    KUrl tmp( m_url );

    if ( tmp.fileName(KUrl::ObeyTrailingSlash).isEmpty() )
        tmp.setFileName( "index.html" );
    KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewer::slotStarted(KIO::Job* job)
{
    m_part->widget()->setCursor( Qt::WaitCursor );
    emit started(job);
}

void ArticleViewer::slotCompleted()
{
    m_part->widget()->unsetCursor();
    emit completed();
}

void ArticleViewer::connectToNode(TreeNode* node)
{
    if (node)
    {
        if (m_viewMode == CombinedView)
        {
            connect( node, SIGNAL(signalChanged(Akregator::TreeNode*)), this, SLOT(slotUpdateCombinedView()) );
            connect( node, SIGNAL(signalArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)), this, SLOT(slotArticlesAdded(Akregator::TreeNode*,QList<Akregator::Article>)));
            connect( node, SIGNAL(signalArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)), this, SLOT(slotArticlesRemoved(Akregator::TreeNode*,QList<Akregator::Article>)));
            connect( node, SIGNAL(signalArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)), this, SLOT(slotArticlesUpdated(Akregator::TreeNode*,QList<Akregator::Article>)));
        }

        connect( node, SIGNAL(signalDestroyed(Akregator::TreeNode*)), this, SLOT(slotClear()) );
    }
}

void ArticleViewer::disconnectFromNode(TreeNode* node)
{
    if (node)
        node->disconnect( this );
}

void ArticleViewer::showArticle( const Akregator::Article& article )
{
    if ( article.isNull() || article.isDeleted() )
    {
        slotClear();
        return;
    }

    m_viewMode = NormalView;
    disconnectFromNode(m_node);
    m_article = article;
    m_node = 0;
    m_link = article.link();
    if (article.feed()->loadLinkedWebsite())
    {
        openUrl(article.link());
    }

    setArticleActionsEnabled(true);
}

bool ArticleViewer::openUrl(const KUrl& url)
{
    if (!m_article.isNull() && m_article.feed()->loadLinkedWebsite())
    {
        return m_part->openUrl(url);
    } else {
        return true;
    }
}

void ArticleViewer::setFilters(const std::vector< shared_ptr<const AbstractMatcher> >& filters )
{
    if ( filters == m_filters )
        return;

    m_filters = filters;

    slotUpdateCombinedView();
}

void ArticleViewer::slotUpdateCombinedView()
{
    if (m_viewMode != CombinedView)
        return;

    if (!m_node)
        return slotClear();


   QString text;

   int num = 0;

   const std::vector< shared_ptr<const AbstractMatcher> >::const_iterator filterEnd = m_filters.end();

   Q_FOREACH( const Article& i, m_articles )
   {
       if ( i.isDeleted() )
           continue;

       if ( std::find_if( m_filters.begin(), m_filters.end(), !bind( &AbstractMatcher::matches, _1, i ) ) != filterEnd )
           continue;

       text += "<p><div class=\"article\">"+m_combinedViewFormatter->formatArticle( i, ArticleFormatter::NoIcon)+"</div><p>";
       ++num;
   }
}

void ArticleViewer::slotArticlesUpdated(TreeNode* /*node*/, const QList<Article>& /*list*/)
{
    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewer::slotArticlesAdded(TreeNode* /*node*/, const QList<Article>& list)
{
    if (m_viewMode == CombinedView) {
        //TODO sort list, then merge
        m_articles << list;
        std::sort( m_articles.begin(), m_articles.end() );
        slotUpdateCombinedView();
    }
}

void ArticleViewer::slotArticlesRemoved(TreeNode* /*node*/, const QList<Article>& list )
{
    Q_UNUSED(list)

    if (m_viewMode == CombinedView) {
        //TODO
        slotUpdateCombinedView();
    }
}

void ArticleViewer::slotClear()
{
    disconnectFromNode(m_node);
    m_node = 0;
    m_article = Article();
    m_articles.clear();
}

void ArticleViewer::showNode(TreeNode* node)
{
    m_viewMode = CombinedView;

    if (node != m_node)
        disconnectFromNode(m_node);

    connectToNode(node);

    m_articles.clear();
    m_article = Article();
    m_node = node;

    delete m_listJob;

    m_listJob = node->createListJob();
    connect( m_listJob, SIGNAL(finished(KJob*)), this, SLOT(slotArticlesListed(KJob*)));
    m_listJob->start();



    slotUpdateCombinedView();
}

void ArticleViewer::slotArticlesListed( KJob* job ) {
    assert( job );
    assert( job == m_listJob );

    TreeNode* node = m_listJob->node();

    if ( job->error() || !node ) {
        if ( !node )
            kWarning() << "Node to be listed is already deleted";
        else
            kWarning() << job->errorText();
        slotUpdateCombinedView();
        return;
    }

    m_articles = m_listJob->articles();
    std::sort( m_articles.begin(), m_articles.end() );

    if (node && !m_articles.isEmpty())
        m_link = m_articles.first().link();
    else
        m_link = KUrl();

    slotUpdateCombinedView();
}

void ArticleViewer::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

QSize ArticleViewer::sizeHint() const
{
    // Increase height a bit so that we can (roughly) read 25 lines of text
    QSize sh = QWidget::sizeHint();
    sh.setHeight(qMax(sh.height(), 25 * fontMetrics().height()));
    return sh;
}

ArticleViewerPart::ArticleViewerPart(QWidget* parent) : KParts::ReadOnlyPart(parent),
     m_button(-1)
{
    setXMLFile(KStandardDirs::locate("data", "akregator/articleviewer.rc"), true);
}

int ArticleViewerPart::button() const
{
    return m_button;
}

bool ArticleViewerPart::closeUrl()
{
    emit browserExtension()->loadingProgress(-1);
    emit canceled(QString());
    return KParts::ReadOnlyPart::closeUrl();
}

bool ArticleViewerPart::urlSelected(const QString &url, int button, int state, const QString &_target,
                                    const KParts::OpenUrlArguments& args,
                                    const KParts::BrowserArguments& browserArgs)
{
    m_button = button;
    if(url == "config:/disable_introduction")
    {
        KGuiItem yesButton(KStandardGuiItem::yes());
        yesButton.setText(i18n("Disable"));
        KGuiItem noButton(KStandardGuiItem::no());
        noButton.setText(i18n("Keep Enabled"));
        if(KMessageBox::questionYesNo( widget(), i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), yesButton, noButton) == KMessageBox::Yes)
        {
            KConfigGroup conf(Settings::self()->config(), "General");
            conf.writeEntry("Disable Introduction", "true");
            conf.sync();
            return true;
        }

        return false;
    }
    // else
    //    return KParts::ReadOnlyPart::urlSelected(url,button,state,_target,args,browserArgs);
    return false;
}

void ArticleViewer::setArticleActionsEnabled(bool enabled)
{
    ActionManager::getInstance()->setArticleActionsEnabled(enabled);
}

} // namespace Akregator


