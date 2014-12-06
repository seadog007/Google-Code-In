/****************************************************************************************
 * Copyright (c) 2013 Anmol Ahuja <darthcodus@gmail.com>                                *
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

#ifndef SCRIPT_EDITOR_DOCUMENT_H
#define SCRIPT_EDITOR_DOCUMENT_H

#include <KSharedPtr>

#include <QObject>
#include <QWeakPointer>

namespace KTextEditor
{
    class Attribute;
    class Document;
    class View;
}
class KUrl;
class QColor;
class QIcon;
class QWidget;

namespace ScriptConsoleNS
{
    class AmarokScriptCodeCompletionModel;

    class ScriptEditorDocument : public QObject
    {
        public:
            ScriptEditorDocument( QObject *parent, KTextEditor::Document* document );
            virtual ~ScriptEditorDocument();
            QString text() const;
            KTextEditor::View *createView( QWidget *editor = 0 );
            void setText( const QString &text );
            void save( const KUrl &url );
            void save();
            void setReadWrite( bool readWrite );
            static void highlight( KTextEditor::View *view, int line, const QColor &color );
            static void clearHighlights( KTextEditor::View *view );

        private:
            KTextEditor::Document *m_document;
            static QWeakPointer<AmarokScriptCodeCompletionModel> s_completionModel;
    };
}

#endif // SCRIPT_EDITOR_DOCUMENT_H