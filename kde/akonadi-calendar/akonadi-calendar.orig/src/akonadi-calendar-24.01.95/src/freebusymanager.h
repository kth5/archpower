/*
  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  SPDX-FileCopyrightText: 2002-2004 Klarälvdalens Datakonsult AB,
        <info@klaralvdalens-datakonsult.se>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "akonadi-calendar_export.h"
#include "etmcalendar.h"

#include <KCalendarCore/FreeBusyCache>

#include <memory>

// TODO: document
namespace Akonadi
{
class FreeBusyManagerPrivate;
class FreeBusyManagerStatic;

class AKONADI_CALENDAR_EXPORT FreeBusyManager : public QObject, public KCalendarCore::FreeBusyCache
{
    Q_OBJECT
public:
    /**
     * Returns the FreeBusyManager
     */
    static FreeBusyManager *self();

    void setCalendar(const Akonadi::ETMCalendar::Ptr &calendar);

    /**
      Publishes the owners freebusy information from the current calendar
      starting from the current date/time to current date/time + freeBusyPublishDays.
      If an upload is already in progress nothing happens.

      @see KCalPrefBase::freeBusyPublishUrl()
      @see KCalPrefBase::freeBusyPublishDays();
      */
    Q_INVOKABLE void publishFreeBusy(QWidget *parentWidget = nullptr);

    /**
      Mail the freebusy information.
      */
    Q_INVOKABLE void mailFreeBusy(int daysToPublish = 30, QWidget *parentWidget = nullptr);

    /**
      Retrieve the freebusy information of somebody else, i.e. it will not try
      to download our own freebusy information.

      This method will first try to find Akonadi resource that have the
      'FreeBusyProvider' capability set. If none is found then there is a fallback
      on the URL mechanism (see below). If at least one is found then it will
      be first queried over D-Bus to know if it can handle free-busy information
      for that email address. If true then it will be queried for the free-busy
      data for a period ranging from today to today plus 14 days, defined in
      FreeBusyManagerPrivate::FreeBusyProvidersRequestsQueue::FreeBusyProvidersRequestsQueue()
      as hard-coded magic value. If the Akonadi resource responds successfully
      (still over D-Bus) then the freeBusyRetrieved signal is emitted. If any
      of those steps then the URL mechanism will be used as a fallback.

      The URL mechanism makes use of a local cache, if the information
      for a given email is already downloaded it will return the information
      from the cache. The free-busy information must be accessible using HTTP
      and the URL is build dynamically from the email address and the global
      groupware settings.

      The call is asynchronous, a download is started in the background (if
      needed) and freeBusyRetrieved will be emitted when the download is finished.

      @see KCalPrefs::thatIsMe( const QString &email );
      @see Akonadi::FreeBusyProviderBase

      @param email Address of the person for which the F/B list should be
              retrieved.
      @param forceDownload Set to true to trigger a download even when automatic
              retrieval of freebusy information is disabled in the configuration.
      @return true if a download is initiated, and false otherwise
    */
    Q_INVOKABLE bool retrieveFreeBusy(const QString &email, bool forceDownload, QWidget *parentWidget = nullptr);

    /**
      Clears the retrieval queue, i.e. all retrieval request that are not started
      yet will not start at all. The freebusy retrieval that currently is
      downloading (if one) will not be canceled.

      @see retrieveFreeBusy
      */
    void cancelRetrieval();

    /**
      Load freebusy information belonging to an email. The information is loaded
      from a local file. If the file does not exists or doesn't contain valid
      information 0 is returned. In that case the information should be retrieved
      again by calling retrieveFreeBusy.

      Implements KCalendarCore::FreeBusyCache::loadFreeBusy

      @param email is a QString containing a email string in the
      "FirstName LastName <emailaddress>" format.
    */
    [[nodiscard]] KCalendarCore::FreeBusy::Ptr loadFreeBusy(const QString &email) override;

    /**
      Save freebusy information belonging to an email.

      Implements KCalendarCore::FreeBusyCache::saveFreeBusy

      @param freebusy is a pointer to a valid FreeBusy instance.
      @param person is a valid Person instance.
    */
    bool saveFreeBusy(const KCalendarCore::FreeBusy::Ptr &freebusy, const KCalendarCore::Person &person) override;

Q_SIGNALS:
    /**
      This signal is emitted to return results of free/busy requests.
    */
    void freeBusyRetrieved(const KCalendarCore::FreeBusy::Ptr &fb, const QString &email);

protected:
    /** React on timer events, used for delayed freebusy list uploading */
    void timerEvent(QTimerEvent *event) override;

private:
    /**
      Creates a new FreeBusyManager, private because FreeBusyManager is a
      Singleton
      */
    AKONADI_CALENDAR_NO_EXPORT FreeBusyManager();
    ~FreeBusyManager() override;

private:
    friend class FreeBusyManagerStatic;

    std::unique_ptr<FreeBusyManagerPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(FreeBusyManager)
    Q_DISABLE_COPY(FreeBusyManager)
    Q_PRIVATE_SLOT(d_ptr, void checkFreeBusyUrl())
    Q_PRIVATE_SLOT(d_ptr, void processFreeBusyDownloadResult(KJob *))
    Q_PRIVATE_SLOT(d_ptr, void processFreeBusyUploadResult(KJob *))
    Q_PRIVATE_SLOT(d_ptr, void processRetrieveQueue())
    Q_PRIVATE_SLOT(d_ptr, void uploadFreeBusy())
};
}
