/*
 * This file is part of crash-reporter
 *
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: Jakub Adam <jakub.adam@jollamobile.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "crashreporteradapter.h"

#include <QFileSystemWatcher>

#include "creportercoreregistry.h"
#include "creporterutils.h"
#include "pendinguploadsmodel.h"

class CrashReporterAdapterPrivate
{
public:
    CrashReporterAdapterPrivate(CrashReporterAdapter *qq);

    int reportsToUpload;
    PendingUploadsModel pendingUploadsModel;

private:
    void updateCoreDirectoryModels();

    QFileSystemWatcher watcher;

    Q_DECLARE_PUBLIC(CrashReporterAdapter)
    CrashReporterAdapter *q_ptr;
};

CrashReporterAdapterPrivate::CrashReporterAdapterPrivate(CrashReporterAdapter *qq)
    : reportsToUpload(0), q_ptr(qq)
{
    Q_Q(CrashReporterAdapter);

    updateCoreDirectoryModels();
    // Recalculate the models when directory contents change.
    QObject::connect(&watcher, SIGNAL(directoryChanged(const QString &)),
                     q, SLOT(updateCoreDirectoryModels()));

    watcher.addPaths(CReporterCoreRegistry::instance()->getCoreLocationPaths());
}

void CrashReporterAdapterPrivate::updateCoreDirectoryModels()
{
    Q_Q(CrashReporterAdapter);

    QStringList coreFiles(CReporterCoreRegistry::instance()->collectAllCoreFiles());

    int newCoresToUpload = coreFiles.count();
    if (newCoresToUpload != reportsToUpload) {
        reportsToUpload = newCoresToUpload;
        emit q->reportsToUploadChanged();
    }

    pendingUploadsModel.setData(coreFiles);
}

CrashReporterAdapter::CrashReporterAdapter(QObject *parent)
    : QObject(parent), d_ptr(new CrashReporterAdapterPrivate(this))
{
}

int CrashReporterAdapter::reportsToUpload() const
{
    Q_D(const CrashReporterAdapter);

    return d->reportsToUpload;
}

QAbstractListModel *CrashReporterAdapter::pendingUploads()
{
    Q_D(CrashReporterAdapter);

    return &d->pendingUploadsModel;
}

void CrashReporterAdapter::deleteCrashReport(const QString &filePath) const
{
    QFile::remove(filePath);
}

void CrashReporterAdapter::uploadAllCrashReports() const
{
    CReporterCoreRegistry *registry = CReporterCoreRegistry::instance();
    CReporterUtils::notifyAutoUploader(registry->collectAllCoreFiles(), false);
}

void CrashReporterAdapter::deleteAllCrashReports() const
{
    foreach (const QString &filename, CReporterCoreRegistry::instance()->collectAllCoreFiles()) {
        QFile(filename).remove();
    }
}

#include "moc_crashreporteradapter.cpp"
