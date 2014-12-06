/****************************************************************************************
 * Copyright (c) 2010 Maximilian Kossick <maximilian.kossick@googlemail.com>            *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_PROXY_LOGGER_H
#define AMAROK_PROXY_LOGGER_H

#include "core/interfaces/Logger.h"

#include <QMetaType>
#include <QMutex>
#include <QPair>
#include <QWeakPointer>
#include <QQueue>
#include <QTimer>

#include <KJob>

class QNetworkReply;

typedef QPair<QString, Amarok::Logger::MessageType> LongMessage;

struct ProgressData
{
    QWeakPointer<QObject> sender;
    QWeakPointer<KJob> job;
    QWeakPointer<QNetworkReply> reply;
    QString text;
    int maximum;
    QWeakPointer<QObject> cancelObject;
    const char *slot;
    Qt::ConnectionType type;
};

/**
  * Proxy implementation for the Amarok::Logger interface.
  * This class does not notify the user, but forwards the notifications
  * to a real logger if available. If no logger is available yet, it stores
  * the notifications until another logger becomes available.
  *
  * This class can be only instantiated from the main thread and must reside in the main
  * thread for its lifetime.
  */
class ProxyLogger : public QObject, public Amarok::Logger
{
    Q_OBJECT
    Q_PROPERTY( Amarok::Logger* logger
                READ logger
                WRITE setLogger
                DESIGNABLE false )

public:
    ProxyLogger();
    virtual ~ProxyLogger();

public slots:
    virtual void shortMessage( const QString &text );
    virtual void longMessage( const QString &text, MessageType type );
    virtual void newProgressOperation( KJob *job, const QString &text, QObject *obj = 0,
                                       const char *slot = 0,
                                       Qt::ConnectionType type = Qt::AutoConnection );
    virtual void newProgressOperation( QNetworkReply *reply, const QString &text, QObject *obj = 0,
                                       const char *slot = 0,
                                       Qt::ConnectionType type = Qt::AutoConnection );
    virtual void newProgressOperation( QObject *sender, const QString &text, int maximum = 100,
                                       QObject *obj = 0, const char *slot = 0,
                                       Qt::ConnectionType type = Qt::AutoConnection );

    /**
      * Set the real logger.
      * The proxy logger will forward notifications to this logger.
      * @param logger The real logger to use. ProxyLogger does not take ownership of the pointer
      */
    void setLogger( Logger *logger );
    Logger* logger() const;

private slots:
    void forwardNotifications();
    void slotStartTimer();
    void slotTotalSteps( int totalSteps );

signals:
    // timer can only be started from its thread, use signals & slots to pass thread barrier
    void startTimer();

private:
    Logger *m_logger; //!< stores the real logger
    QMutex m_lock; //!< protect members that may be accessed from multiple threads
    QTimer *m_timer; //!< internal timer that triggers forwarding of notifications
    QQueue<QString> m_shortMessageQueue; //!< temporary storage for notifications that have not been forwarded yet
    QQueue<LongMessage> m_longMessageQueue; //!< temporary storage for notifications that have not been forwarded yet
    QQueue<ProgressData> m_progressQueue; //!< temporary storage for notifications that have not been forwarded yet
};

Q_DECLARE_METATYPE(ProxyLogger *)
#endif
