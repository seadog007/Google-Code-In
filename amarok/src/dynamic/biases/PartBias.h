/****************************************************************************************
 * Copyright (c) 2008 Daniel Jones <danielcjones@gmail.com>                             *
 * Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>                                    *
 * Copyright (c) 2010, 2011 Ralf Engels <ralf-engels@gmx.de>                                  *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) version 3 or        *
 * any later version accepted by the membership of KDE e.V. (or its successor approved  *
 * by the membership of KDE e.V.), which shall act as a proxy defined in Section 14 of  *
 * version 3 of the license.                                                            *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef AMAROK_PARTBIAS_H
#define AMAROK_PARTBIAS_H

#include "dynamic/Bias.h"
#include "dynamic/BiasFactory.h"

#include <QList>
#include <QVector>
class QGridLayout;
class QSlider;
class QWidget;

namespace Dynamic
{
    class PartBias;

    /** The widget for the PartBias */
    class PartBiasWidget : public QWidget
    {
        Q_OBJECT

        public:
            PartBiasWidget( Dynamic::PartBias* bias, QWidget* parent = 0 );

        protected slots:
            void biasAppended( Dynamic::BiasPtr bias );
            void biasRemoved( int pos );
            void biasMoved( int from, int to );

            void sliderValueChanged( int val );
            void biasWeightsChanged();

        protected:
            /** True if we just handle a signal. Used to protect agains recursion */
            bool m_inSignal;

            Dynamic::PartBias* m_bias;

            QGridLayout* m_layout;

            QList<QSlider*> m_sliders;
            QList<QWidget*> m_widgets;
    };

    /** The part bias will ensure that tracks are fulfilling all the sub-biases according to it's weights.
        The bias has an implicit random sub-bias
    */
    class PartBias : public AndBias
    {
        Q_OBJECT

        public:
            /** Create a new part bias.
                @param empty If true, then the newly created bias will not have a set of example sub-biases.
            */
            PartBias();

            virtual void fromXml( QXmlStreamReader *reader );
            virtual void toXml( QXmlStreamWriter *writer ) const;

            static QString sName();
            virtual QString name() const;
            virtual QString toString() const;

            virtual QWidget* widget( QWidget* parent = 0 );
            virtual void paintOperator( QPainter* painter, const QRect &rect, Dynamic::AbstractBias* bias );

            virtual TrackSet matchingTracks( const Meta::TrackList& playlist,
                                             int contextCount, int finalCount,
                                             const TrackCollectionPtr universe ) const;

            virtual bool trackMatches( int position,
                                       const Meta::TrackList& playlist,
                                       int contextCount ) const;

            /** Returns the weights of the bias itself and all the sub-biases. */
            virtual QList<qreal> weights() const;

            /** Appends a bias to this bias.
                This object will take ownership of the bias and free it when destroyed.
            */
            virtual void appendBias( Dynamic::BiasPtr bias );
            virtual void moveBias( int from, int to );

        public slots:
            void resultReceived( const Dynamic::TrackSet &tracks );

            /** The overall weight has changed */
            void changeBiasWeight( int biasNum, qreal value );

        signals:
            /** The overall weight has changed */
            void weightsChanged();

        protected slots:
            virtual void biasReplaced( Dynamic::BiasPtr oldBias, Dynamic::BiasPtr newBias );

        private:
            /** Using the data from m_matchingTracks it tries to compute a valid m_tracks */
            void updateResults() const; // only changes mutables

            QList<qreal> m_weights;
            mutable QVector<Dynamic::TrackSet> m_matchingTracks;

            // buffered by matchingTracks
            mutable Meta::TrackList m_playlist;
            mutable int m_contextCount;
            mutable int m_finalCount;
            mutable Dynamic::TrackCollectionPtr m_universe;

            Q_DISABLE_COPY(PartBias)
    };


    class AMAROK_EXPORT PartBiasFactory : public Dynamic::AbstractBiasFactory
    {
        public:
            virtual QString i18nName() const;
            virtual QString name() const;
            virtual QString i18nDescription() const;
            virtual BiasPtr createBias();
    };

}


#endif

