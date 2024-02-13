/**
  This file is part of the akonadi-calendar library.

  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <Akonadi/Collection>

#include <QObject>
#include <QString>

#include <memory>

/**
 * A class to import ical calendar files into akonadi.
 *
 * @since 4.12
 */
namespace Akonadi
{
class IncidenceChanger;
class ICalImporterPrivate;

class AKONADI_CALENDAR_EXPORT ICalImporter : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new ICalImporter object. Use a different ICalImporter instance for each file you want to import.
     *
     * @param changer An existing IncidenceChanger, if 0, an internal one will be created.
     *                If you pass an existing one, you will be able to undo/redo import operations.
     * @param parent  Parent QObject.
     */
    explicit ICalImporter(Akonadi::IncidenceChanger *changer = nullptr, QObject *parent = nullptr);
    ~ICalImporter() override;

    /**
     * Translated error message.
     * This is set when either importIntoExistingFinished() or importIntoNewResource() return false
     * or when the corresponding signals are have success=false.
     */
    [[nodiscard]] QString errorMessage() const;

Q_SIGNALS:
    /**
     * Emitted after calling importIntoExistingResource()
     * @param success Success of the operation.
     * @param total Number of incidences included in the ical file.
     *
     * @see importIntoExistingResource(), errorMessage().
     */
    void importIntoExistingFinished(bool success, int total);

    /**
     * Emitted after calling importIntoNewResource().
     * If success is false, check errorMessage().
     */
    void importIntoNewFinished(bool success);

public Q_SLOTS:

    /**
     * Creates a new akonadi_ical_resource and configures it to use @p filename.
     * @param filename ical absolute file path
     * @return True if the job was started, in this case you should wait for the corresponding signal.
     */
    bool importIntoNewResource(const QString &filename);

    /**
     * Imports an ical file into an existing resource.
     * @param url Path of a local or remote file to import.
     * @param collectionId The destination collection. If null, the user will be prompted for a destination.
     *
     * @return false if some basic validation happened, like insufficient permissions. Use errorMessage() to see
     *         what happened. The importIntoExistingFinished() signal won't be emitted in this case.
     *
     *         true if the import job was started. importIntoExistingFinished() signal will be emitted in this case.
     */
    bool importIntoExistingResource(const QUrl &url, Akonadi::Collection collection);

private:
    std::unique_ptr<ICalImporterPrivate> const d;
};
}
