/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#ifndef Q_CUPS_H
#define Q_CUPS_H

#include <kdemacros.h>
#include <QObject>
#include <QHash>

#define DEST_IDLE     '3'
#define DEST_PRINTING '4'
#define DEST_STOPED   '5'

namespace QCups
{
    class KDE_EXPORT Printer: public QObject
    {
        Q_OBJECT
    public:
        Printer(QObject *parent = 0);
        Printer(const QString &destName, QObject *parent = 0);

        QString value(const QString &name) const;

        bool save(QHash<QString, QVariant> values);

        static bool setShared(const QString &destName, bool shared);
        static QHash<QString, QVariant> getAttributes(const QString &destName, const QStringList &requestedAttr);
        static bool setAttributesFile(const QString &destName, const QStringList &requestedAttr);

    private:
        QString m_destName;
        QHash<QString, QString> m_values;
    };

    KDE_EXPORT void initialize();
    KDE_EXPORT bool cancelJob(const QString &name, int job_id);
    KDE_EXPORT bool holdJob(const QString &name, int job_id);
    KDE_EXPORT bool releaseJob(const QString &name, int job_id);
    KDE_EXPORT bool moveJob(const QString &name, int job_id, const QString &dest_name);
    KDE_EXPORT bool pausePrinter(const QString &name);
    KDE_EXPORT bool resumePrinter(const QString &name);
    KDE_EXPORT bool setDefaultPrinter(const QString &name);
    KDE_EXPORT bool deletePrinter(const QString &name);
    KDE_EXPORT bool addModifyPrinter(const QString &name, const QHash<QString, QVariant> values);
};

#endif
