/****************************************************************************************
 * Copyright (c) 2011 Bart Cerneels <bart.cerneels@kde.org                              *
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

#ifndef BROWSERMESSAGEAREA_H
#define BROWSERMESSAGEAREA_H

#include "core-impl/logger/ProxyLogger.h"
#include "statusbar/CompoundProgressBar.h"

#include <QFrame>
#include <QTimer>

class BrowserMessageArea : public QFrame, public Amarok::Logger
{
    Q_OBJECT

public:
    BrowserMessageArea( QWidget *parent );

    ~BrowserMessageArea()
    {
    }

    /* Amarok::Logger virtual methods */
    virtual void shortMessage( const QString &text );
    virtual void longMessage( const QString &text, MessageType type );

    virtual void newProgressOperation( KJob *job, const QString &text, QObject *obj,
                                      const char *slot, Qt::ConnectionType type );

    virtual void newProgressOperation( QNetworkReply *reply, const QString &text, QObject *obj,
                                      const char *slot, Qt::ConnectionType type );

    virtual void newProgressOperation( QObject *sender, const QString &text, int maximum,
                                       QObject *obj, const char *slot, Qt::ConnectionType type );
signals:
    void signalLongMessage( const QString & text, MessageType type );

private slots:
    void hideProgress();
    void nextShortMessage();
    void hideLongMessage();
    void slotLongMessage( const QString &text, MessageType type = Information );

private:
    CompoundProgressBar *m_progressBar;
    QLabel *m_messageLabel;

    bool m_busy;
    QTimer *m_shortMessageTimer;
    QList<QString> m_shortMessageQueue;
};

#endif // BROWSERMESSAGEAREA_H
