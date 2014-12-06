/****************************************************************************************
 * Copyright (c) 2008 Seb Ruiz <ruiz@kde.org>                                           *
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

#ifndef AMAROK_COVERVIEWDIALOG_H
#define AMAROK_COVERVIEWDIALOG_H

#include "amarok_export.h"
#include "core/meta/forward_declarations.h"

#include <QDialog>

class AMAROK_EXPORT CoverViewDialog : public QDialog
{
    Q_OBJECT

    public:
        CoverViewDialog( Meta::AlbumPtr album, QWidget *parent );
        CoverViewDialog( const QImage &image, QWidget *parent );

    private slots:
        void updateCaption();
        void zoomFactorChanged( qreal value );

    private:
        void createViewer( const QImage &image, const QWidget *widget );

        QString m_title;
        QSize m_size;
        int m_zoom;
};

#endif
