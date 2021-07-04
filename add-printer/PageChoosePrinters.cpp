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

#include "PageChoosePrinters.h"
#include "ui_PageChoosePrinters.h"

#include <ClassListWidget.h>

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>

PageChoosePrinters::PageChoosePrinters(const QVariantHash &args, QWidget *parent) :
    GenericPage(parent),
    ui(new Ui::PageChoosePrinters)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    // loads the standard key icon

    const int printerSize = 128;
    const int overlaySize = 48;

    QPixmap printerIcon = QIcon::fromTheme(QStringLiteral("printer")).pixmap(printerSize);
    const QPixmap preferencesIcon = QIcon::fromTheme(QStringLiteral("preferences-other")).pixmap(overlaySize);

    QPainter painter(&printerIcon);

    // bottom right corner
    const QPoint startPoint = QPoint(printerSize - overlaySize - 2,
                                     printerSize - overlaySize - 2);
    painter.drawPixmap(startPoint, preferencesIcon);
    ui->printerL->setPixmap(printerIcon);

    connect(ui->membersLV, static_cast<void (ClassListWidget::*) (bool)>(&ClassListWidget::changed),
            this, &PageChoosePrinters::allowProceed);

    if (!args.isEmpty()) {
        setValues(args);
    }
}

PageChoosePrinters::~PageChoosePrinters()
{
    delete ui;
}

void PageChoosePrinters::setValues(const QVariantHash &args)
{
    if (m_args != args) {
        m_args = args;
    }
}

QVariantHash PageChoosePrinters::values() const
{
    QVariantHash ret = m_args;
    ret[KCUPS_MEMBER_URIS] = ui->membersLV->currentSelected(true);
    return ret;
}

bool PageChoosePrinters::canProceed() const
{
    return ui->membersLV->selectedPrinters().count() > 0;
}

#include "moc_PageChoosePrinters.cpp"
