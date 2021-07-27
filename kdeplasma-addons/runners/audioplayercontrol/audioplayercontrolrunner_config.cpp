/*
 *   Copyright (C) 2009 Jan G. Marker <jangerrit@weiler-marker.com>
 *   Copyright (C) 2021 Ivailo Monev <xakepa10@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//Project-Includes
#include "audioplayercontrolrunner_config.h"

//KDE-Includes
#include <Plasma/AbstractRunner>
#include <KDebug>

#include "audioplayercontrolconfigkeys.h"

K_EXPORT_RUNNER_CONFIG(audioplayercontrol, AudioPlayerControlRunnerConfig)

AudioPlayerControlRunnerConfigForm::AudioPlayerControlRunnerConfigForm(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

AudioPlayerControlRunnerConfig::AudioPlayerControlRunnerConfig(QWidget* parent, const QVariantList& args) :
        KCModule(AudioPlayerControlRunnerConfigFactory::componentData(), parent, args)
{
    m_ui = new AudioPlayerControlRunnerConfigForm(this);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    connect(m_ui->player_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(changed()));
    connect(m_ui->player_combo, SIGNAL(editTextChanged(QString)), this, SLOT(changed()));
    connect(m_ui->play_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->append_edit, SIGNAL(textChanged(QString)),this,SLOT(changed()));
    connect(m_ui->pause_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->next_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->prev_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->stop_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->volume_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(m_ui->quit_edit, SIGNAL(textChanged(QString)), this, SLOT(changed()));


    /* Setup the combobox to select the player */
    m_ui->player_combo->setDuplicatesEnabled(false);
    m_ui->player_combo->addItem(i18n("VLC"), QLatin1String("vlc"));
    m_ui->player_combo->addItem(i18n("Audacious"), QLatin1String("audacious"));
    /* template for adding other players:
     * m_ui->player_combo->addItem(i18n("player's name"), "the end of it's MPRIS interface");
     */
    m_ui->player_combo->setEditable(true); //Let's add our own players

    load();
}

void AudioPlayerControlRunnerConfig::load()
{
    KCModule::load();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners");
    grp = KConfigGroup(&grp, "Audio Player Control Runner");
    QString player = grp.readEntry(CONFIG_PLAYER, "vlc");
    int index = m_ui->player_combo->findData(player);
    if (index == -1) {
        m_ui->player_combo->addItem(player, player);
        m_ui->player_combo->setCurrentItem(player);
    } else {
        m_ui->player_combo->setCurrentIndex(index);
    }
    m_ui->play_edit->setText(grp.readEntry(CONFIG_PLAY , i18n("play")));
    m_ui->append_edit->setText(grp.readEntry(CONFIG_APPEND, i18n("append")));
    m_ui->pause_edit->setText(grp.readEntry(CONFIG_PAUSE, i18n("pause")));
    m_ui->next_edit->setText(grp.readEntry(CONFIG_NEXT, i18nc("next song", "next")));
    m_ui->prev_edit->setText(grp.readEntry(CONFIG_PREV , i18nc("previous song", "prev")));
    m_ui->stop_edit->setText(grp.readEntry(CONFIG_STOP , i18n("stop")));
    m_ui->volume_edit->setText(grp.readEntry(CONFIG_VOLUME , i18nc("set the sound volume", "volume")));
    m_ui->quit_edit->setText(grp.readEntry(CONFIG_QUIT, i18nc("quit media player", "quit")));

    emit changed(false);
}

void AudioPlayerControlRunnerConfig::save()
{
    KCModule::save();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig(QLatin1String("krunnerrc"));
    KConfigGroup grp = cfg->group("Runners");
    grp = KConfigGroup(&grp, "Audio Player Control Runner");
    grp.writeEntry(CONFIG_PLAY, m_ui->play_edit->text());
    grp.writeEntry(CONFIG_APPEND, m_ui->append_edit->text());
    grp.writeEntry(CONFIG_PAUSE, m_ui->pause_edit->text());
    grp.writeEntry(CONFIG_STOP, m_ui->stop_edit->text());
    grp.writeEntry(CONFIG_PREV, m_ui->prev_edit->text());
    grp.writeEntry(CONFIG_NEXT, m_ui->next_edit->text());
    grp.writeEntry(CONFIG_VOLUME, m_ui->volume_edit->text());
    grp.writeEntry(CONFIG_QUIT, m_ui->quit_edit->text());
    QString data = m_ui->player_combo->itemData(m_ui->player_combo->findText(
                       m_ui->player_combo->currentText())).toString();
    if (data.isEmpty()) {
        grp.writeEntry(CONFIG_PLAYER, m_ui->player_combo->currentText());
    } else {
        grp.writeEntry(CONFIG_PLAYER, data);
    }
    grp.sync();

    emit changed(false);
}

void AudioPlayerControlRunnerConfig::defaults()
{
    KCModule::defaults();

    m_ui->player_combo->setCurrentIndex(m_ui->player_combo->findData(QLatin1String("vlc")));
    m_ui->play_edit->setText(i18n("Play"));
    m_ui->append_edit->setText(i18n("Append"));
    m_ui->pause_edit->setText(i18n("Pause")) ;
    m_ui->next_edit->setText(i18n("Next"));
    m_ui->prev_edit->setText(i18n("Prev"));
    m_ui->stop_edit->setText(i18n("Stop"));
    m_ui->volume_edit->setText(i18n("Volume"));
    m_ui->quit_edit->setText(i18n("Quit"));

    emit changed(true);
}
