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

#include "PrintQueue.h"

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KAboutData about("PrintQueue",
                     "print-queue",
                     ki18n("PrintQueue"),
                     "0.1",
                     ki18n("PrintQueue"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2009 Daniel Nicoletti"));

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti85-pk@yahoo.com.br");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("show-queue [queue name]", ki18n("Show printer queue"));
    KCmdLineArgs::addCmdLineOptions(options);

    PrintQueue::addCmdLineOptions();

    if (!PrintQueue::start()) {
        //kDebug() << "PrintQueue is already running!";
        return 0;
    }

    PrintQueue app;
    return app.exec();
}
