/*
    SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>

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

#ifndef NOSELECTIONRECTDELEGATEEGATE_H
#define NOSELECTIONRECTDELEGATEEGATE_H

#include <QStyledItemDelegate>

class Q_DECL_EXPORT NoSelectionRectDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NoSelectionRectDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // NOSELECTIONRECTDELEGATEEGATE_H
