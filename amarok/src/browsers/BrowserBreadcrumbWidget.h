/****************************************************************************************
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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

#ifndef BROWSERBREADCRUMBWIDGET_H
#define BROWSERBREADCRUMBWIDGET_H

#include <KHBox>
#include <KLineEdit>

#include <KPushButton>
#include <QList>
#include <QStackedWidget>
#include <QStringList>

class BreadcrumbItemMenuButton;
class BrowserBreadcrumbItem;
class BrowserCategoryList;

/**
 *  A widget for displaying the current state of and navigating the category dig down interface.
 *
 *	@author Nikolaj Hald Nielsen <nhn@kde.org>
 */
class BrowserBreadcrumbWidget : public KHBox
{
    Q_OBJECT
public:

    /**
     * Constructor
     * @param parent the parent widget
     */
    BrowserBreadcrumbWidget( QWidget * parent );

    /**
     * Destructor
     * @param parent the parent widget
     */
    ~BrowserBreadcrumbWidget();

    /**
     * Set the BrowserCategoryList which acts as the "root" of the breadcrumb widget.
     * A root breadcrumb item is created that represents the lowest level, and the categories
     * in the list are added to the items dropdown menu.
     * @param rootList the BrowserCategoryList representing the lowest level in the navigation hirachy
     */
    void setRootList( BrowserCategoryList *rootList );

signals:
    /**
     * Signal emitted when the root breadcrumb item is clicked.
     */
    void toHome();

public slots:
    /**
     * Rebuild the list of breadcrumb items corrosponding to the current location in the hirachy.
     * This also allows for categories that add additional breadcrumb items (such as the file browser) to update the
     * breadcrumbs when their internal state changes.
     */
    void updateBreadcrumbs();

protected:
    virtual void resizeEvent( QResizeEvent * event );

private slots:
    /**
     * Goes through all breadcrumb items and shows the most relevant ones based on
     * available size. (always shows home icon and the last item)
     */
    void showAsNeeded();

private:
    /**
     * Remove all breadcrumb items
     */
    void clearCrumbs();

    /**
     * Recursive function that traverses the tree of BrowserCategoryList's
     * and adds each one as a level in the breadcrumb.
     * @param level the root level BrowserCategoryList.
     */
    void addLevel( BrowserCategoryList *list );

    /**
     * Helper function for addLevel() that first hides BrowserBreadcrumbItem, adds it to
     * to breadcrumb area.
     */
    void addBreadCrumbItem( BrowserBreadcrumbItem *item );

    //QStringList m_currentPath;
    BrowserCategoryList * m_rootList;

    QList<BrowserBreadcrumbItem *> m_items;
    QWidget *m_spacer;
    KHBox *m_breadcrumbArea;

    BreadcrumbItemMenuButton *m_childMenuButton;

};

#endif
