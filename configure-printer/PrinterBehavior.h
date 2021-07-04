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

#ifndef PRINTER_BEHAVIOR_H
#define PRINTER_BEHAVIOR_H

#include "PrinterPage.h"

#include "KCupsRequest.h"
#include <QWidget>

namespace Ui {
    class PrinterBehavior;
}

class PrinterBehavior : public PrinterPage
{
    Q_OBJECT
public:
    explicit PrinterBehavior(const QString &destName, bool isClass, QWidget *parent = nullptr);
    ~PrinterBehavior() override;

    void setValues(const KCupsPrinter &printer);
    void setRemote(bool remote) override;
    bool hasChanges() override;

    QStringList neededValues() const override;
    void save() override;

private slots:
    void currentIndexChangedCB(int index);
    void userListChanged();

private:
    QString errorPolicyString(const QString &policy) const;
    QString operationPolicyString(const QString &policy) const;
    QString jobSheetsString(const QString &policy) const;

    Ui::PrinterBehavior *ui;
    QString m_destName;
    bool m_isClass;
    QVariantHash m_changedValues;
    int m_changes = 0;
};

#endif
