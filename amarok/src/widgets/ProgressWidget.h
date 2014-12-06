/****************************************************************************************
 * Copyright (c) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>                        *
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

#ifndef AMAROK_PROGRESSWIDGET_H
#define AMAROK_PROGRESSWIDGET_H

#include "core/meta/forward_declarations.h"

#include <Phonon/Global>

#include <QHash>
#include <QPainter>
#include <QPolygon>
#include <QWidget>

class TimeLabel;
namespace Amarok { class TimeSlider; }

class ProgressWidget : public QWidget
{
    Q_OBJECT

    public:
        ProgressWidget( QWidget* );

        virtual QSize sizeHint() const;
        void addBookmark( const QString &name, int milliSeconds , bool instantDisplayPopUp );
        Amarok::TimeSlider* slider() const { return m_slider; }

    public slots:
        void drawTimeDisplay( int position );

    protected slots:
        void stopped();
        void paused();
        void trackPlaying();
        void trackLengthChanged( qint64 milliseconds );
        void trackPositionChanged( qint64 position );

    protected:
        virtual void mousePressEvent( QMouseEvent * );

    private slots:
        void addBookmark( const QString &name, int milliSeconds );
        void redrawBookmarks(const QString *BookmarkName = 0);

    private:
        void updateTimeLabelTooltips();

        TimeLabel *m_timeLabelLeft;
        TimeLabel *m_timeLabelRight;
        Amarok::TimeSlider *m_slider;
        QString m_currentUrlId;
};

#endif

