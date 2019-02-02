/***************************************************************************
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "KCupsPasswordDialog.h"

#include "Debug.h"

#include <QPointer>

#include <KPasswordDialog>
#include <KWindowSystem>
#include <KLocalizedString>

KCupsPasswordDialog::KCupsPasswordDialog(QObject *parent) :
    QObject(parent),
    m_accepted(false),
    m_mainwindow(0),
    // default text, can be overriden using setPromptText()
    m_promptText(i18n("Enter an username and a password to complete the task"))
{
}

void KCupsPasswordDialog::setMainWindow(WId mainwindow)
{
    m_mainwindow = mainwindow;
}

void KCupsPasswordDialog::setPromptText(const QString &text)
{
    m_promptText = text;
}

void KCupsPasswordDialog::exec(const QString &username, bool wrongPassword)
{
    QPointer<KPasswordDialog> dialog = new KPasswordDialog(nullptr, KPasswordDialog::ShowUsernameLine);
    dialog->setPrompt(m_promptText);
    dialog->setModal(false);
    dialog->setUsername(username);
    if (wrongPassword) {
        dialog->showErrorMessage(QString(), KPasswordDialog::UsernameError);
        dialog->showErrorMessage(i18n("Wrong username or password"), KPasswordDialog::PasswordError);
    }

    dialog->show();
    if (m_mainwindow) {
        KWindowSystem::setMainWindow(dialog, m_mainwindow);
    }
    KWindowSystem::forceActiveWindow(dialog->winId());

    // Do not return from this method now
    dialog->exec();

    if (dialog) {
        m_accepted = dialog->result() == QDialog::Accepted;
        m_username = dialog->username();
        m_password = dialog->password();
        dialog->deleteLater();
    }
}

bool KCupsPasswordDialog::accepted() const
{
    return m_accepted;
}

QString KCupsPasswordDialog::username() const
{
    return m_username;
}

QString KCupsPasswordDialog::password() const
{
    return m_password;
}

#include "moc_KCupsPasswordDialog.cpp"
