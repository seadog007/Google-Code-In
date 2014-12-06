/****************************************************************************************
 * Copyright (c) 2012 Matěj Laitl <matej@laitl.cz>                                      *
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

#include "ExcludedLabelsDialog.h"

#include "core/meta/Meta.h"
#include "core/collections/QueryMaker.h"
#include "core-impl/collections/support/CollectionManager.h"
#include "statsyncing/Config.h"

#include <KLineEdit>
#include <KLocalizedString>

#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QToolButton>

ExcludedLabelsDialog::ExcludedLabelsDialog( StatSyncing::Config *config, QWidget *parent,
                                            Qt::WFlags flags )
    : KDialog( parent, flags )
    , m_statSyncingConfig( config )
{
    Q_ASSERT( m_statSyncingConfig );
    setupUi( mainWidget() );
    setButtons( Ok | Cancel );
    setCaption( i18n( "Excluded Labels" ) );

    addLabels( config->excludedLabels(), true );
    Collections::QueryMaker *qm = CollectionManager::instance()->queryMaker();
    qm->setQueryType( Collections::QueryMaker::Label );
    qm->setAutoDelete( true );
    connect( qm, SIGNAL(newResultReady(Meta::LabelList)),
             SLOT(slowNewResultReady(Meta::LabelList)) );
    qm->run();

    connect( addButton, SIGNAL(clicked(bool)), SLOT(slotAddExcludedLabel()) );
    connect( addLabelLine, SIGNAL(returnPressed(QString)), SLOT(slotAddExcludedLabel()) );
    connect( this, SIGNAL(okClicked()), SLOT(slotSaveToConfig()) );
}

void
ExcludedLabelsDialog::slowNewResultReady( const Meta::LabelList &labels )
{
    foreach( const Meta::LabelPtr &label, labels )
        addLabel( label->name() );
}

void
ExcludedLabelsDialog::slotAddExcludedLabel()
{
    addLabel( addLabelLine->text(), true );
    addLabelLine->setText( QString() );
}

void
ExcludedLabelsDialog::slotSaveToConfig()
{
    QSet<QString> excluded;
    foreach( const QListWidgetItem *item, listWidget->selectedItems() )
    {
        excluded.insert( item->text() );
    }
    m_statSyncingConfig->setExcludedLabels( excluded );
}

void
ExcludedLabelsDialog::addLabel( const QString &label, bool selected )
{
    int count = listWidget->count();
    for( int i = 0; i <= count; i++ )
    {
        QModelIndex idx;
        if( i == count )
        {
            // reached end of the list
            listWidget->addItem( label );
            idx = listWidget->model()->index( i, 0 );
        }
        else if( listWidget->item( i )->text() == label )
        {
            // already in list
            return;
        }
        else if( QString::localeAwareCompare( listWidget->item( i )->text(), label ) > 0 )
        {
            listWidget->insertItem( i, label );
            idx = listWidget->model()->index( i, 0 );
        }
        // else continue to next iteration

        // just inserted
        if( idx.isValid() && selected )
            listWidget->selectionModel()->select( idx, QItemSelectionModel::Select );
        if( idx.isValid() )
            return;
    }
}

void
ExcludedLabelsDialog::addLabels( const QSet<QString> &labels, bool selected )
{
    foreach( const QString &label, labels )
    {
        addLabel( label, selected );
    }
}
