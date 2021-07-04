/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti
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

#ifndef CONFIGURE_DIALOG_H
#define CONFIGURE_DIALOG_H

#include <QAbstractButton>
#include <QCloseEvent>
#include <KPageDialog>

class PrinterPage;
class ModifyPrinter;
class PrinterOptions;

class Q_DECL_EXPORT ConfigureDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit ConfigureDialog(const QString &destName, bool isClass, QWidget *parent = nullptr);
    ~ConfigureDialog() override;

private:
    void currentPageChangedSlot(KPageWidgetItem *current, KPageWidgetItem *before);
    void enableButtonApply(bool enable);
    void slotButtonClicked(QAbstractButton * pressedButton);
    void ppdChanged();

    ModifyPrinter *modifyPrinter;
    PrinterOptions *printerOptions;
    void closeEvent(QCloseEvent *event) override;
    // return false if the dialog was canceled
    bool savePage(PrinterPage *page);
};

#endif
