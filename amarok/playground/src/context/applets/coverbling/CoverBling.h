/****************************************************************************************
 * Copyright (c) 2007 Mark Kretschmann <kretschmann@kde.org>                            *
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

#ifndef AMAROK_COVERBLING_H
#define AMAROK_COVERBLING_H

#include "core/meta/forward_declarations.h"

#include <QGLWidget>


class CoverBling : public QGLWidget
{
    Q_OBJECT

    public:
        CoverBling( QWidget* parent );

    protected:
        void initializeGL();
        void resizeGL( int width, int height );
        void setPerspective();
        void paintGL();
        void draw( GLuint selected = 0 );
        GLuint objectAtPosition( const QPoint& pos );

    private slots:
        void queryResult( Meta::AlbumList albums );

    private:
        QList<QPixmap> m_covers;
        QList<GLuint> m_textureIds;
        GLuint m_texturedRectList;
        GLuint m_texturedRectReflectedList;
        float m_xOffset;
        float m_zOffset;
};


#endif /* AMAROK_COVERBLING_H */


