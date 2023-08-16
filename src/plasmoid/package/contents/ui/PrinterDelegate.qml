/*
    SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2014-2015 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
    SPDX-FileCopyrightText: 2023 Mike Noe <noeerover@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 
import QtQuick.Controls
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami 2 as Kirigami

PlasmaExtras.ExpandableListItem {
    readonly property bool isPaused: model.printerState === 5

    icon: model.iconName
    iconEmblem: isPaused ? "emblem-pause" : ""
    title: model.printerName
    subtitle: model.stateMessage
    isDefault: model.isDefault

    defaultActionButtonAction: Kirigami.Action {
        icon.name: isPaused ? "media-playback-start" : "media-playback-pause"
        text: isPaused ? i18n("Resume printing") : i18n("Pause printing")
        onTriggered: {
            if (isPaused) {
                printersModel.resumePrinter(model.printerName);
            } else {
                printersModel.pausePrinter(model.printerName);
            }
        }
    }

    contextualActionsModel: [
        Kirigami.Action {
            icon.name: "configure"
            text: i18n("Configure printer...")
            onTriggered: processRunner.configurePrinter(model.printerName);
        },
        Kirigami.Action {
            icon.name: "view-list-details"
            text: i18n("View print queue")
            onTriggered: processRunner.openPrintQueue(model.printerName);
        }
    ]
}
