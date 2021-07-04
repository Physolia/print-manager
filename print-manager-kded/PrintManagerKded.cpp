/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti
    dantti12@gmail.com

    SPDX-License-Identifier: GPL-2.0-or-later

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "PrintManagerKded.h"


#include "NewPrinterNotification.h"

K_PLUGIN_CLASS_WITH_JSON(PrintManagerKded, "printmanager.json")

PrintManagerKded::PrintManagerKded(QObject *parent, const QVariantList &args) :
    KDEDModule(parent)
{
    Q_UNUSED(args)

    new NewPrinterNotification(this);
}

PrintManagerKded::~PrintManagerKded()
{
}

#include "PrintManagerKded.moc"
