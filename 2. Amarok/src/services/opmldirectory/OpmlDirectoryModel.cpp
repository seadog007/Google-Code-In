/****************************************************************************************
 * Copyright (c) 2010 Bart Cerneels <bart.cerneels@kde.org                              *
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

#include "OpmlDirectoryModel.h"

#include "core/support/Amarok.h"
#include "MainWindow.h"
#include "OpmlParser.h"
#include "OpmlWriter.h"
#include "core/support/Debug.h"
//included to access defaultPodcasts()
#include "playlistmanager/PlaylistManager.h"
#include "core/podcasts/PodcastProvider.h"

#include "ui_AddOpmlWidget.h"

#include <ThreadWeaver/Weaver>

#include <KDialog>

#include <QAction>

OpmlDirectoryModel::OpmlDirectoryModel( KUrl outlineUrl, QObject *parent )
    : QAbstractItemModel( parent )
    , m_rootOpmlUrl( outlineUrl )
{
    //fetchMore will be called by the view
    m_addOpmlAction = new QAction( KIcon( "list-add" ), i18n( "Add OPML" ), this );
    connect( m_addOpmlAction, SIGNAL(triggered()), SLOT(slotAddOpmlAction()) );

    m_addFolderAction = new QAction( KIcon( "folder-add" ), i18n( "Add Folder"), this );
    connect( m_addFolderAction, SIGNAL(triggered()), SLOT(slotAddFolderAction()) );
}

OpmlDirectoryModel::~OpmlDirectoryModel()
{
}

QModelIndex
OpmlDirectoryModel::index( int row, int column, const QModelIndex &parent ) const
{
    if( !parent.isValid() )
    {
        if( m_rootOutlines.isEmpty() || m_rootOutlines.count() <= row )
            return QModelIndex();
        else
            return createIndex( row, column, m_rootOutlines[row] );
    }

    OpmlOutline *parentOutline = static_cast<OpmlOutline *>( parent.internalPointer() );
    if( !parentOutline )
        return QModelIndex();

    if( !parentOutline->hasChildren() || parentOutline->children().count() <= row )
        return QModelIndex();

    return createIndex( row, column, parentOutline->children()[row] );
}

Qt::ItemFlags
OpmlDirectoryModel::flags( const QModelIndex &idx ) const
{
    if( !idx.isValid() )
        return Qt::ItemIsDropEnabled;

    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );
    if( outline && !outline->attributes().contains( "type" ) ) //probably a folder
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled
                | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags( idx );
}

QModelIndex
OpmlDirectoryModel::parent( const QModelIndex &idx ) const
{
    if( !idx.isValid() )
        return QModelIndex();
    debug() << idx;
    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );
    if( outline->isRootItem() )
        return QModelIndex();

    OpmlOutline *parentOutline = outline->parent();
    int childIndex;
    if( parentOutline->isRootItem() )
        childIndex = m_rootOutlines.indexOf( parentOutline );
    else
        childIndex = parentOutline->parent()->children().indexOf( parentOutline );
    return createIndex( childIndex, 0, parentOutline );
}

int
OpmlDirectoryModel::rowCount( const QModelIndex &parent ) const
{
    if( !parent.isValid() )
        return m_rootOutlines.count();

    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );

    if( !outline || !outline->hasChildren() )
        return 0;
    else
        return outline->children().count();
}

bool
OpmlDirectoryModel::hasChildren( const QModelIndex &parent ) const
{
    debug() << parent;
    if( !parent.isValid() )
        return !m_rootOutlines.isEmpty();

    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );

    if( !outline )
        return false;

    if( outline->hasChildren() )
        return true;

    return outline->attributes().value( "type" ) == "include";
}

int
OpmlDirectoryModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant
OpmlDirectoryModel::data( const QModelIndex &idx, int role ) const
{
    if( !idx.isValid() )
    {
        if( role == ActionRole )
        {
            QList<QAction *> actions;
            actions << m_addOpmlAction << m_addFolderAction;
            return QVariant::fromValue( actions );
        }
        return QVariant();
    }

    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );
    if( !outline )
        return QVariant();

    switch( role )
    {
        case Qt::DisplayRole:
            return outline->attributes()["text"];
        case Qt::DecorationRole:
            return m_imageMap.contains( outline ) ? m_imageMap.value( outline ) : QVariant();
        case ActionRole:
        {
            if( outline->opmlNodeType() == RegularNode ) //probably a folder
            {
                //store the index the new item should get added to
                m_addOpmlAction->setData( QVariant::fromValue( idx ) );
                m_addFolderAction->setData( QVariant::fromValue( idx ) );
                return QVariant::fromValue( QActionList() << m_addOpmlAction << m_addFolderAction );
            }
        }
        default:
            return QVariant();
    }

    return QVariant();
}

bool
OpmlDirectoryModel::setData( const QModelIndex &idx, const QVariant &value, int role )
{
    Q_UNUSED(role);

    if( !idx.isValid() )
        return false;

    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );
    if( !outline )
        return false;

    outline->mutableAttributes()["text"] = value.toString();

    saveOpml( m_rootOpmlUrl );

    return true;
}

bool
OpmlDirectoryModel::removeRows( int row, int count, const QModelIndex &parent )
{
    if( !parent.isValid() )
    {
        if( m_rootOutlines.count() >= ( row + count ) )
        {
            beginRemoveRows( parent, row, row + count - 1 );
            for( int i = 0; i < count; i++ )
                m_rootOutlines.removeAt( row );
            endRemoveRows();
            saveOpml( m_rootOpmlUrl );
            return true;
        }

        return false;
    }

    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );
    if( !outline )
        return false;

    if( !outline->hasChildren() || outline->children().count() < ( row + count ) )
        return false;

    beginRemoveRows( parent, row, row + count -1 );
    for( int i = 0; i < count - 1; i++ )
            outline->mutableChildren().removeAt( row );
    endRemoveRows();

    saveOpml( m_rootOpmlUrl );

    return true;
}

void
OpmlDirectoryModel::saveOpml( const KUrl &saveLocation )
{
    if( !saveLocation.isLocalFile() )
    {
        //TODO:implement
        error() << "can not save OPML to remote location";
        return;
    }

    QFile *opmlFile = new QFile( saveLocation.toLocalFile(), this );
    if( !opmlFile->open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
        error() << "could not open OPML file for writing " << saveLocation.url();
        return;
    }

    QMap<QString,QString> headerData;
    //TODO: set header data such as date

    OpmlWriter *opmlWriter = new OpmlWriter( m_rootOutlines, headerData, opmlFile );
    connect( opmlWriter, SIGNAL(result(int)), SLOT(slotOpmlWriterDone(int)) );
    opmlWriter->run();
}

void
OpmlDirectoryModel::slotOpmlWriterDone( int result )
{
    Q_UNUSED( result )

    OpmlWriter *writer = qobject_cast<OpmlWriter *>( QObject::sender() );
    Q_ASSERT( writer );
    writer->device()->close();
    delete writer;
}

OpmlNodeType
OpmlDirectoryModel::opmlNodeType( const QModelIndex &idx ) const
{
    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );
    return outline->opmlNodeType();
}

void
OpmlDirectoryModel::slotAddOpmlAction()
{
    QModelIndex parentIdx = QModelIndex();
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action )
    {
        parentIdx = action->data().value<QModelIndex>();
    }

    KDialog *dialog = new KDialog( The::mainWindow() );
    dialog->setCaption( i18nc( "Heading of Add OPML dialog", "Add OPML" ) );
    dialog->setButtons( KDialog::Ok | KDialog::Cancel );
    QWidget *opmlAddWidget = new QWidget( dialog );
    Ui::AddOpmlWidget widget;
    widget.setupUi( opmlAddWidget );
    widget.urlEdit->setMode( KFile::File );
    dialog->setMainWidget( opmlAddWidget );

    if( dialog->exec() != QDialog::Accepted )
        return;

    QString url = widget.urlEdit->url().url();
    QString title = widget.titleEdit->text();
    debug() << QString( "creating a new OPML outline with url = %1 and title \"%2\"." ).arg( url, title );
    OpmlOutline *outline = new OpmlOutline();
    outline->addAttribute( "type", "include" );
    outline->addAttribute( "url", url );
    if( !title.isEmpty() )
        outline->addAttribute( "text", title );

    //Folder icon with down-arrow emblem
    m_imageMap.insert( outline, KIcon( "folder", 0, QStringList( "go-down" ) ).pixmap( 24, 24 ) );

    QModelIndex newIdx = addOutlineToModel( parentIdx, outline );
    //TODO: force the view to expand the folder (parentIdx) so the new node is shown

    //if the title is missing, start parsing the OPML so we can get it from the feed
    if( outline->attributes().contains( "text" ) )
        saveOpml( m_rootOpmlUrl );
    else
        fetchMore( newIdx ); //saves OPML after receiving the title.

    delete dialog;
}

void
OpmlDirectoryModel::slotAddFolderAction()
{
    QModelIndex parentIdx = QModelIndex();
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action )
    {
        parentIdx = action->data().value<QModelIndex>();
    }

    OpmlOutline *outline = new OpmlOutline();
    outline->addAttribute( "text", i18n( "New Folder" ) );
    m_imageMap.insert( outline, KIcon( "folder" ).pixmap( 24, 24 ) );

    addOutlineToModel( parentIdx, outline );
    //TODO: trigger edit of the new folder

    saveOpml( m_rootOpmlUrl );
}

bool
OpmlDirectoryModel::canFetchMore( const QModelIndex &parent ) const
{
    debug() << parent;
    //already fetched or just started?
    if( rowCount( parent ) || m_currentFetchingMap.values().contains( parent ) )
        return false;
    if( !parent.isValid() )
        return m_rootOutlines.isEmpty();

    OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );

    return outline && ( outline->attributes().value( "type" ) == "include" );
}

void
OpmlDirectoryModel::fetchMore( const QModelIndex &parent )
{
    debug() << parent;
    if( m_currentFetchingMap.values().contains( parent ) )
    {
        error() << "trying to start second fetch job for same item";
        return;
    }
    KUrl urlToFetch;
    if( !parent.isValid() )
    {
        urlToFetch = m_rootOpmlUrl;
    }
    else
    {
        OpmlOutline *outline = static_cast<OpmlOutline *>( parent.internalPointer() );
        if( !outline )
            return;
        if( outline->attributes().value( "type" ) != "include" )
            return;
        urlToFetch = KUrl( outline->attributes()["url"] );
    }

    if( !urlToFetch.isValid() )
        return;

    OpmlParser *parser = new OpmlParser( urlToFetch );
    connect( parser, SIGNAL(headerDone()), SLOT(slotOpmlHeaderDone()) );
    connect( parser, SIGNAL(outlineParsed(OpmlOutline*)),
             SLOT(slotOpmlOutlineParsed(OpmlOutline*)) );
    connect( parser, SIGNAL(doneParsing()), SLOT(slotOpmlParsingDone()) );

    m_currentFetchingMap.insert( parser, parent );

//    ThreadWeaver::Weaver::instance()->enqueue( parser );
    parser->run();
}

void
OpmlDirectoryModel::slotOpmlHeaderDone()
{
    OpmlParser *parser = qobject_cast<OpmlParser *>( QObject::sender() );
    QModelIndex idx = m_currentFetchingMap.value( parser );

    if( !idx.isValid() ) //header data of the root not required.
        return;

    OpmlOutline *outline = static_cast<OpmlOutline *>( idx.internalPointer() );

    if( !outline->attributes().contains("text") )
    {
        if( parser->headerData().contains( "title" ) )
            outline->addAttribute( "text", parser->headerData()["title"] );
        else
            outline->addAttribute( "text", parser->url().fileName() );

        //force a view update
        emit dataChanged( idx, idx );

        saveOpml( m_rootOpmlUrl );
    }

}

void
OpmlDirectoryModel::slotOpmlOutlineParsed( OpmlOutline *outline )
{
    OpmlParser *parser = qobject_cast<OpmlParser *>( QObject::sender() );
    QModelIndex idx = m_currentFetchingMap.value( parser );

    addOutlineToModel( idx, outline );

    //TODO: begin image fetch
    switch( outline->opmlNodeType() )
    {
        case RegularNode:
            m_imageMap.insert( outline, KIcon( "folder" ).pixmap( 24, 24 ) ); break;
        case IncludeNode:
        {
            m_imageMap.insert( outline,
                               KIcon( "folder", 0, QStringList( "go-down" ) ).pixmap( 24, 24 )
                             );
            break;
        }
        case RssUrlNode:
        default: break;
    }
}

void
OpmlDirectoryModel::slotOpmlParsingDone()
{
    OpmlParser *parser = qobject_cast<OpmlParser *>( QObject::sender() );
    m_currentFetchingMap.remove( parser );
    parser->deleteLater();
}

void
OpmlDirectoryModel::subscribe( const QModelIndexList &indexes ) const
{
    QList<OpmlOutline *> outlines;

    foreach( const QModelIndex &idx, indexes )
        outlines << static_cast<OpmlOutline *>( idx.internalPointer() );

    foreach( const OpmlOutline *outline, outlines )
    {
        if( !outline )
            continue;

        KUrl url;
        if( outline->attributes().contains( "xmlUrl" ) )
            url = KUrl( outline->attributes()["xmlUrl"] );
        else if( outline->attributes().contains( "url" ) )
            url = KUrl( outline->attributes()["url"] );

        if( url.isEmpty() )
            continue;

        The::playlistManager()->defaultPodcasts()->addPodcast( url );
    }
}

QModelIndex
OpmlDirectoryModel::addOutlineToModel( QModelIndex parentIdx, OpmlOutline *outline )
{
    int newRow = rowCount( parentIdx );
    beginInsertRows( parentIdx, newRow, newRow );

    //no reparenting required when the item is already parented.
    if( outline->isRootItem() )
    {
        if( parentIdx.isValid() )
        {
            OpmlOutline * parentOutline = static_cast<OpmlOutline *>( parentIdx.internalPointer() );
            Q_ASSERT(parentOutline);

            outline->setParent( parentOutline );
            parentOutline->addChild( outline );
            parentOutline->setHasChildren( true );
        }
        else
        {
            m_rootOutlines << outline;
        }
    }
    endInsertRows();

    return index( newRow, 0, parentIdx );
}
