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

#include "KCupsRequest.h"

#include "KCupsJob.h"
#include "KCupsPrinter.h"
#include "KCupsServer.h"

#include <KLocale>
#include <KDebug>

#include <cups/adminutil.h>

#define CUPS_DATADIR    "/usr/share/cups"

KCupsRequest::KCupsRequest() :
    m_finished(true),
    m_error(false)
{
    connect(this, SIGNAL(finished()), &m_loop, SLOT(quit()));
    qRegisterMetaType<KCupsJob>("KCupsJob");
    qRegisterMetaType<KCupsJob::Attributes>("KCupsJob::Attributes");
    qRegisterMetaType<KCupsPrinter>("KCupsPrinter");
    qRegisterMetaType<KCupsPrinter::Attributes>("KCupsPrinter::Attributes");
    qRegisterMetaType<KCupsServer>("KCupsServer");
}

QString KCupsRequest::serverError() const
{
    switch (error()) {
    case IPP_SERVICE_UNAVAILABLE:
        return i18n("Service is unavailable");
    case IPP_NOT_FOUND :
        return i18n("Not found");
    default : // In this case we don't want to map all enums
        kWarning() << "status unrecognised: " << error();
        return QString();
    }
}

void KCupsRequest::getPPDS(const QString &make)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        if (!make.isEmpty()){
            request["ppd-make-and-model"] = make;
        }
        request["need-dest-name"] = false;

        m_retArguments = KCupsConnection::request(CUPS_GET_PPDS,
                                                  "/",
                                                  request,
                                                  true);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getDevices");
    }
}

static void
choose_device_cb(
    const char *device_class,           /* I - Class */
    const char *device_id,              /* I - 1284 device ID */
    const char *device_info,            /* I - Description */
    const char *device_make_and_model,  /* I - Make and model */
    const char *device_uri,             /* I - Device URI */
    const char *device_location,        /* I - Location */
    void *user_data)                    /* I - Result object */
{
    /*
     * Add the device to the array...
     */
    KCupsRequest *request = static_cast<KCupsRequest*>(user_data);
    QMetaObject::invokeMethod(request,
                              "device",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromUtf8(device_class)),
                              Q_ARG(QString, QString::fromUtf8(device_id)),
                              Q_ARG(QString, QString::fromUtf8(device_info)),
                              Q_ARG(QString, QString::fromUtf8(device_make_and_model)),
                              Q_ARG(QString, QString::fromUtf8(device_uri)),
                              Q_ARG(QString, QString::fromUtf8(device_location)));
}

void KCupsRequest::getDevices()
{
    if (KCupsConnection::readyToStart()) {
        do {
            // Scan for devices for 30 seconds
            // TODO change back to 30
            cupsGetDevices(CUPS_HTTP_DEFAULT,
                           5,
                           CUPS_INCLUDE_ALL,
                           CUPS_EXCLUDE_NONE,
                           (cups_device_cb_t) choose_device_cb,
                           this);
        } while (KCupsConnection::retryIfForbidden());
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getDevices");
    }
}

// THIS function can get the default server dest through the
// "printer-is-default" attribute BUT it does not get user
// defined default printer, see cupsGetDefault() on www.cups.org for details

void KCupsRequest::getPrinters(KCupsPrinter::Attributes attributes, cups_ptype_t mask)
{
    QVariantHash arguments;
    arguments["printer-type-mask"] = mask;
    getPrinters(attributes, arguments);
}

void KCupsRequest::getPrinters(KCupsPrinter::Attributes attributes, const QVariantHash &arguments)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request = arguments;
        request["printer-type"] = CUPS_PRINTER_LOCAL;
        request["requested-attributes"] = KCupsPrinter::flags(attributes);
        request["need-dest-name"] = true;

        ReturnArguments dests;
        dests = KCupsConnection::request(CUPS_GET_PRINTERS,
                                         "/",
                                         request,
                                         true);

        for (int i = 0; i < dests.size(); i++) {
            m_printers << KCupsPrinter(dests.at(i));
        }

        m_retArguments = dests;
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getPrinters", qVariantFromValue(attributes), arguments);
    }
}

void KCupsRequest::getJobs(const QString &printer, bool myJobs, int whichJobs, KCupsJob::Attributes attributes)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        // This makes the Name of the Job and owner came blank lol
//        if (printer.isEmpty()) {
//            request["printer-uri"] = printer;
//        } else {
            request["printer-name"] = printer;
//        }

        if (myJobs) {
            request["my-jobs"] = myJobs;
        }

        if (whichJobs == CUPS_WHICHJOBS_COMPLETED) {
            request["which-jobs"] = "completed";
        } else if (whichJobs == CUPS_WHICHJOBS_ALL) {
            request["which-jobs"] = "all";
        }

        QStringList attributesStrList = KCupsJob::flags(attributes);
        if (!attributesStrList.isEmpty()) {
            request["requested-attributes"] = attributesStrList;
        }
        request["group-tag-qt"] = IPP_TAG_JOB;

        m_retArguments = KCupsConnection::request(IPP_GET_JOBS,
                                                  "/",
                                                  request,
                                                  true);

        for (int i = 0; i < m_retArguments.size(); i++) {
            m_jobs << KCupsJob(m_retArguments.at(i));
        }

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getJobs", printer, myJobs, whichJobs, qVariantFromValue(attributes));
    }
}

void KCupsRequest::addClass(const QHash<QString, QVariant> &values)
{
//    if (values.isEmpty()) {
//        return 0;
//    }

    if (KCupsConnection::readyToStart()) {
        QVariantHash request = values;
        request["printer-is-class"] = true;
        request["printer-is-accepting-jobs"] = true;
        request["printer-state"] = IPP_PRINTER_IDLE;
        m_retArguments = KCupsConnection::request(CUPS_ADD_MODIFY_CLASS,
                                                  "/admin/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("addClass", values);
    }
}

void KCupsRequest::getServerSettings()
{
    if (KCupsConnection::readyToStart()) {
        do {
            int num_settings;
            cups_option_t *settings;
            QVariantHash arguments;
            cupsAdminGetServerSettings(CUPS_HTTP_DEFAULT, &num_settings, &settings);
            for (int i = 0; i < num_settings; ++i) {
                QString name = QString::fromUtf8(settings[i].name);
                QString value = QString::fromUtf8(settings[i].value);
                arguments[name] = value;
            }
            cupsFreeOptions(num_settings, settings);

            emit server(KCupsServer(arguments));
        } while (KCupsConnection::retryIfForbidden());
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getServerSettings");
    }
}

void KCupsRequest::setServerSettings(const KCupsServer &server)
{
    if (KCupsConnection::readyToStart()) {
        do {
            QVariantHash args = server.arguments();
            int num_settings = 0;
            cups_option_t *settings;

            QVariantHash::const_iterator i = args.constBegin();
            while (i != args.constEnd()) {
                num_settings = cupsAddOption(i.key().toUtf8(),
                                             i.value().toString().toUtf8(),
                                             num_settings,
                                             &settings);
                ++i;
            }

            cupsAdminSetServerSettings(CUPS_HTTP_DEFAULT, num_settings, settings);
            cupsFreeOptions(num_settings, settings);
        } while (KCupsConnection::retryIfForbidden());
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setServerSettings", qVariantFromValue(server));
    }
}

void KCupsRequest::setAttributes(const QString &printer, bool isClass, const QVariantHash &values, const char *filename)
{
    if (values.isEmpty() && !filename) {
        setFinished();
        return;
    }

    if (KCupsConnection::readyToStart()) {
        QVariantHash request = values;
        request["printer-name"] = printer;
        request["printer-is-class"] = isClass;
        if (filename) {
            request["filename"] = filename;
        }

        ipp_op_e op;
        // TODO this seems weird now.. review this code..
        if (isClass && values.contains("member-uris")) {
            op = CUPS_ADD_CLASS;
        } else {
            op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;
        }


        m_retArguments = KCupsConnection::request(op,
                                                  "/admin/",
                                                  request,
                                                  false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setAttributes", printer, isClass, values, filename);
    }
}

void KCupsRequest::setShared(const QString &printer, bool isClass, bool shared)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = printer;
        request["printer-is-class"] = isClass;
        request["printer-is-shared"] = shared;
        request["need-dest-name"] = true;

        ipp_op_e op = isClass ? CUPS_ADD_MODIFY_CLASS : CUPS_ADD_MODIFY_PRINTER;

        m_retArguments = KCupsConnection::request(op,
                                                  "/admin/",
                                                  request,
                                                  false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setShared", printer, isClass, shared);
    }
}

void KCupsRequest::pausePrinter(const QString &name)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = name;

        KCupsConnection::request(IPP_PAUSE_PRINTER,
                                 "/admin/",
                                 request,
                                 false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("pausePrinter", name);
    }
}

void KCupsRequest::resumePrinter(const QString &name)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = name;

        KCupsConnection::request(IPP_RESUME_PRINTER,
                                 "/admin/",
                                 request,
                                 false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("resumePrinter", name);
    }
}

void KCupsRequest::setDefaultPrinter(const QString &name)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = name;

        KCupsConnection::request(CUPS_SET_DEFAULT,
                                 "/admin/",
                                 request,
                                 false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("setDefaultPrinter", name);
    }
}

void KCupsRequest::deletePrinter(const QString &name)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = name;

        KCupsConnection::request(CUPS_DELETE_PRINTER,
                                 "/admin/",
                                 request,
                                 false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("deletePrinter", name);
    }
}

void KCupsRequest::getAttributes(const QString &printer, bool isClass, const QStringList &requestedAttr)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = printer;
        request["printer-is-class"] = isClass;
        request["need-dest-name"] = false; // we don't need a dest name since it's a single list
        request["requested-attributes"] = requestedAttr;

        m_retArguments = KCupsConnection::request(IPP_GET_PRINTER_ATTRIBUTES,
                                                  "/admin/",
                                                  request,
                                                  true);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("getAttributes", printer, isClass, requestedAttr);
    }
}

void KCupsRequest::printTestPage(const QString &printer, bool isClass)
{
    if (KCupsConnection::readyToStart()) {
        QVariantHash request;
        request["printer-name"] = printer;
        request["printer-is-class"] = isClass;
        request["job-name"] = i18n("Test Page");
        char          resource[1024], /* POST resource path */
                      filename[1024]; /* Test page filename */
        const char    *datadir;       /* CUPS_DATADIR env var */

        /*
         * Locate the test page file...
         */
        datadir = qgetenv("CUPS_DATADIR").isEmpty() ? CUPS_DATADIR : qgetenv("CUPS_DATADIR") ;
        snprintf(filename, sizeof(filename), "%s/data/testprint", datadir);
        request["filename"] = filename;

        /*
         * Point to the printer/class...
         */
        snprintf(resource, sizeof(resource),
                 isClass ? "/classes/%s" : "/printers/%s", printer.toUtf8().data());

        m_retArguments = KCupsConnection::request(IPP_PRINT_JOB,
                                                  resource,
                                                  request,
                                                  false);

        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printTestPage", printer, isClass);
    }
}

void KCupsRequest::printCommand(const QString &printer, const QString &command, const QString &title)
{
    if (KCupsConnection::readyToStart()) {
        do {
            int           job_id;                 /* Command file job */
            char          command_file[1024];     /* Command "file" */
            http_status_t status;                 /* Document status */
            cups_option_t hold_option;            /* job-hold-until option */

            /*
             * Create the CUPS command file...
             */
            snprintf(command_file, sizeof(command_file), "#CUPS-COMMAND\n%s\n", command.toUtf8().data());

            /*
             * Send the command file job...
             */
            hold_option.name  = const_cast<char*>("job-hold-until");
            hold_option.value = const_cast<char*>("no-hold");

            if ((job_id = cupsCreateJob(CUPS_HTTP_DEFAULT,
                                        printer.toUtf8(),
                                        title.toUtf8(),
                                        1,
                                        &hold_option)) < 1) {
                qWarning() << "Unable to send command to printer driver!";

                setError(IPP_NOT_POSSIBLE, i18n("Unable to send command to printer driver!"));
                setFinished();
                return;
            }

            status = cupsStartDocument(CUPS_HTTP_DEFAULT,
                                       printer.toUtf8(),
                                       job_id,
                                       NULL,
                                       CUPS_FORMAT_COMMAND,
                                       1);
            if (status == HTTP_CONTINUE) {
                status = cupsWriteRequestData(CUPS_HTTP_DEFAULT, command_file,
                                            strlen(command_file));
            }

            if (status == HTTP_CONTINUE) {
                cupsFinishDocument(CUPS_HTTP_DEFAULT, printer.toUtf8());
            }

            setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
            if (cupsLastError() >= IPP_REDIRECTION_OTHER_SITE) {
                qWarning() << "Unable to send command to printer driver!";

                cupsCancelJob(printer.toUtf8(), job_id);
                setFinished();
                return; // Return to avoid a new try
            }
        } while (KCupsConnection::retryIfForbidden());
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("printCommand", printer, command, title);
    }
}

void KCupsRequest::cancelJob(const QString &destName, int jobId)
{
    if (KCupsConnection::readyToStart()) {
        do {
            cupsCancelJob(destName.toUtf8(), jobId);
            setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        } while (KCupsConnection::retryIfForbidden());
        setFinished();
    } else {
        invokeMethod("cancelJob", destName, jobId);
    }
}

void KCupsRequest::holdJob(const QString &destName, int jobId)
{
    if (KCupsConnection::readyToStart()) {
        QHash<QString, QVariant> request;
        request["printer-name"] = destName;
        request["job-id"] = jobId;
        m_retArguments = KCupsConnection::request(IPP_HOLD_JOB,
                                                  "/jobs/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("holdJob", destName, jobId);
    }
}

void KCupsRequest::releaseJob(const QString &destName, int jobId)
{
    if (KCupsConnection::readyToStart()) {
        QHash<QString, QVariant> request;
        request["printer-name"] = destName;
        request["job-id"] = jobId;
        m_retArguments = KCupsConnection::request(IPP_RELEASE_JOB,
                                                  "/jobs/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("releaseJob", destName, jobId);
    }
}

void KCupsRequest::moveJob(const QString &fromDestname, int jobId, const QString &toDestname)
{
    if (jobId < -1 || fromDestname.isEmpty() || toDestname.isEmpty() || jobId == 0) {
        qWarning() << "Internal error, invalid input data" << jobId << fromDestname << toDestname;
        setFinished();
        return;
    }

    if (KCupsConnection::readyToStart()) {
        QHash<QString, QVariant> request;
        request["printer-name"] = fromDestname;
        request["job-id"] = jobId;
        request["job-printer-uri"] = toDestname;

        m_retArguments = KCupsConnection::request(CUPS_MOVE_JOB,
                                                  "/jobs/",
                                                  request,
                                                  false);
        setError(cupsLastError(), QString::fromUtf8(cupsLastErrorString()));
        setFinished();
    } else {
        invokeMethod("moveJob", fromDestname, jobId, toDestname);
    }
}

void KCupsRequest::invokeMethod(const char *method,
                                    const QVariant &arg1,
                                    const QVariant &arg2,
                                    const QVariant &arg3,
                                    const QVariant &arg4,
                                    const QVariant &arg5,
                                    const QVariant &arg6,
                                    const QVariant &arg7,
                                    const QVariant &arg8)
{
    m_error = false;
    m_errorMsg.clear();
    m_printers.clear();
    m_jobs.clear();
    m_retArguments.clear();

    // If this fails we get into a infinite loop
    // Do not use global()->thread() which point
    // to the KCupsConnection parent thread
    moveToThread(KCupsConnection::global());

    m_finished = !QMetaObject::invokeMethod(this,
                                            method,
                                            Qt::QueuedConnection,
                                            QGenericArgument(arg1.typeName(), arg1.data()),
                                            QGenericArgument(arg2.typeName(), arg2.data()),
                                            QGenericArgument(arg3.typeName(), arg3.data()),
                                            QGenericArgument(arg4.typeName(), arg4.data()),
                                            QGenericArgument(arg5.typeName(), arg5.data()),
                                            QGenericArgument(arg6.typeName(), arg6.data()),
                                            QGenericArgument(arg7.typeName(), arg7.data()),
                                            QGenericArgument(arg8.typeName(), arg8.data()));
    if (m_finished) {
        setError(1, i18n("Failed to invoke method: %1").arg(method));
        setFinished();
    }
}

ReturnArguments KCupsRequest::result() const
{
    return m_retArguments;
}

KCupsRequest::KCupsPrinters KCupsRequest::printers() const
{
    return m_printers;
}

KCupsRequest::KCupsJobs KCupsRequest::jobs() const
{
    return m_jobs;
}

void KCupsRequest::waitTillFinished()
{
    kDebug() << QThread::currentThreadId();
    kDebug() << m_finished;
    if (m_finished) {
        return;
    }

    m_loop.exec();
}

bool KCupsRequest::hasError() const
{
    return m_error;
}

int KCupsRequest::error() const
{
    return m_error;
}

QString KCupsRequest::errorMsg() const
{
    return m_errorMsg;
}

void KCupsRequest::setError(int error, const QString &errorMsg)
{
    m_error = error;
    m_errorMsg = errorMsg;
}

void KCupsRequest::setFinished()
{
    m_finished = true;
    emit finished();
}