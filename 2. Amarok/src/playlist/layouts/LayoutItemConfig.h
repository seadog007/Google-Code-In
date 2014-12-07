/****************************************************************************************
 * Copyright (c) 2008 Nikolaj Hald Nielsen <nhn@kde.org>                                *
 * Copyright (c) 2010 Oleksandr Khayrullin <saniokh@gmail.com>                          *
 * Copyright (c) 2010 Nanno Langstraat <langstr@gmail.com>                              *
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

#ifndef LAYOUTITEMCONFIG_H
#define LAYOUTITEMCONFIG_H

#include <QList>
#include <QModelIndex>
#include <QString>

namespace Playlist {

enum
{
    ITEM_LEFT,
    ITEM_RIGHT,
    ITEM_CENTER
};

/**
 * This class corresponds to a single element in a single row in a playlist layout
 * @author Nikolaj Hald Nielsen <nhn@kde.org>
 */
class LayoutItemConfigRowElement
{
    public:
        /**
         * Constructor.
         * @param value An integer representing the information that this item is to show.
         * @param size The percentage size of the row that this item should use. If 0, this
         * item shares the space leftover by any non 0 items with all other items with size 0 on the same row.
         * @param bold Make the item text bold.
         * @param italic Make the item text italic.
         * @param underline Make the item text underline.
         * @param alignment the alignment of the item (ITEM_LEFT, ITEM_RIGHT or ITEM_CENTER).
         * @param prefix Text to show before the actual value text.
         * @param suffix  Text to show after the actual value text.
         */
        LayoutItemConfigRowElement( int value, qreal size,
                                    bool bold, bool italic, bool underline,
                                    Qt::Alignment alignment,
                                    const QString &prefix = QString(),
                                    const QString &suffix = QString() );

        /**
         * Get the value of this element.
         * @return The value.
         */
        int value() const;

        /**
         * Get the percentage of the row that this element should take up.
         * @return The percentage size.
         */
        qreal size() const;

        /**
         * Set the percentage of the row that this element should take up.
         */
        void setSize( qreal size );

        /**
         * Get whether text should be bold.
         * @return Bold or not.
         */
        bool bold() const;

        /**
         * Get whether text should be italic.
         * @return Italic or not.
         */
        bool italic() const;

        /**
         * Get whether text should be underlined.
         * @return Underlined or not.
         */
        bool underline() const;

        /**
         * Get the alignment of this element.
         * @return The alignment.
         */
        Qt::Alignment alignment() const;

        /**
         * Get the prefix. This text is shown before the actual text mandated by value(). For instance, if the value is Artist, a prefix
         * could be "Artist: ". This would make the text in the playlist appear as "Artist: _ARTIST_NAME_"
         * @return The prefix text.
         */
        QString prefix() const;

        /**
         * Get the suffix. This text is shown after the actual text mandated by value(). For instance, if the value is PlayCount, a prefix
         * could be " plays". This would make the text in the playlist appear as "_NO_OF_PLAYS_ plays"
         * @return The suffix text.
         */
        QString suffix() const;

    private:
        int m_value;
        qreal m_size;
        bool m_bold;
        bool m_italic;
        bool m_underline;
        Qt::Alignment m_alignment;
        QString m_prefix, m_suffix;
};

/**
 * This class corrosponds to a  row in a playlist layout
 * @author Nikolaj Hald Nielsen <nhn@kde.org>
 */
class LayoutItemConfigRow
{
    public:

        /**
         * Add an element to the end of this row.
         * @param element The element to add.
         */
        void addElement( LayoutItemConfigRowElement element );

        /**
         * Get the number of elements in this row.
         * @return The element count.
         */
        int count();

        /**
         * Get the element at a specific index.
         * @param at The index of the element.
         * @return The element at the index.
         */
        LayoutItemConfigRowElement element( int at );
    private:
        QList<LayoutItemConfigRowElement> m_elements;
};

/**
 * This class wraps the data needed to paint a LayoutItemDelegate. It knows how many vertical
 * rows there should be, how many items in each row, whether an image should be displayed and so on.
 * @author Nikolaj Hald Nielsen <nhn@kde.org>
 */
class LayoutItemConfig
{
    public:
        /**
         * Constructor.
         */
        LayoutItemConfig();

        /**
         * Destructor.
         */
        ~LayoutItemConfig();

        /**
         * Get the number of rows in this layoutConfig.
         * @return The row count.
         */
        int rows() const;

        /**
         * Get a specific row.
         * @param at The row index.
         * @return The row at the index.
         */
        LayoutItemConfigRow row( int at ) const;

        /**
         * Get whether a cover should be shown.
         * @return Show the cover.
         */
        bool showCover() const;

        /**
         * Get which row should be used to paint the active indicator. This is the graphics that is used to show which track is the
         * currently playing one. For different configs it might make sense to show this on different rows, usually the one containing the track name.
         * @return The row to paint the current track indicator on.
         */
        int activeIndicatorRow() const;

        /**
         * Add a row to this config.
         * @param row The row to add.
         */
        void addRow( LayoutItemConfigRow row );

        /**
         * Set whether the cover image should be shown or not.
         * @param showCover Show the cover image.
         */
        void setShowCover( bool showCover );

        /**
         * Get the row that should be used to paint the current track indicator.
         * @param row The row to show the indicator on.
         */
        void setActiveIndicatorRow( int row );

    private:
        QList<LayoutItemConfigRow> m_rows;
        bool m_showCover;
        int m_activeIndicatorRow;
};

/**
 * This class wraps the data that makes up a playlist layout. A layout consists of 3 LayoutItemConfigs one used for
 * painting single tracks, one used for painting group headers and one used for painting group members.
 * @author Nikolaj Hald Nielsen <nhn@kde.org>
 */
class PlaylistLayout
{
    public:
        enum Part {
            Head = 0,
            StandardBody,
            VariousArtistsBody,
            Single,
            NumParts    // The number of Part values
        };

        /**
        * Default Constructor
        */
        PlaylistLayout();

        /**
         * Get whether this config can be edited/deleted. The default layouts shipped with Amarok are read only,
         * but all user generated layouts can be modified or deleted.
         * @return Can this layout be edited or deleted by the user.
         */
        bool isEditable() const;

        /**
         * Get whether this layout has been changed and needs to be saved.
         * @return Has this layout changed.
         */
        bool isDirty() const;

        /**
         * Determine the layout config for an item in a QAbstractItemModel.
         * Convenience function.
         */
        LayoutItemConfig layoutForItem( const QModelIndex &index ) const { return layoutForPart( partForItem( index ) ); }

        /**
         * Determine the part type for an item in a QAbstractItemModel.
         */
        Part partForItem( const QModelIndex &index ) const;

        /**
         * Get the layout config for the specified part type.
         */
        LayoutItemConfig layoutForPart( Part part ) const;

        /**
         * Set the layout config for the specified part type.
         */
        void setLayoutForPart( Part part, LayoutItemConfig itemConfig );

        /**
         * Set whether this config can be edited by the user.
         * @param editable Can this config be edited.
         */
        void setEditable( bool editable );

        /**
         * Set whether this config has changed and has not yet been saved to file.
         * @param dirty Is this layout dirty.
         */
        void setDirty( bool dirty );

        bool inlineControls();
        void setInlineControls( bool inlineControls );

        bool tooltips();
        void setTooltips( bool tooltips );

        QString groupBy();
        void setGroupBy( const QString & );

    private:
        LayoutItemConfig m_layoutItemConfigs[NumParts];

        bool m_isEditable;
        bool m_isDirty;
        bool m_inlineControls;
        bool m_tooltips;

        QString m_groupBy;
};

}

#endif
