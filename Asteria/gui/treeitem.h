#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QIcon>

/**
 * @brief The TreeItem class
 *
 * Follows the example at http://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
 */
class TreeItem
{
public:
    explicit TreeItem(const QList<QVariant> &data, TreeItem *parentItem=0);
    ~TreeItem();

    void appendChild(TreeItem *item);

    /**
     * Locate the child item stored at the given row
     * @brief child
     * @param row
     * @return
     */
    TreeItem *child(unsigned int row);
    /**
     * Get the number of child items
     * @brief childCount
     * @return
     */
    int childCount() const;

    /**
     * Get the number of columns of data stored in this item
     * @brief columnCount
     * @return
     */
    int columnCount() const;
    /**
     * Get the data stored at the given column
     * @brief data
     * @param column
     * @return
     */
    QVariant data(unsigned int column) const;
    /**
     * Get the row number occupied by this item in it's parent item, or
     * zero if this is the root item.
     * @brief row
     * @return
     */
    int row() const;
    /**
     * Get this item's parent, or NULL if this is the root item.
     * @brief parentItem
     * @return
     */
    TreeItem *parentItem();
    /**
     * Get the icon to be displayed next to this item in the tree viewer
     * @brief getIcon
     * @return
     */
    QIcon getIcon();
    /**
     * Set the icon to be displayed next to this item in the tree viewer
     * @brief setIcon
     * @param iconToUse
     */
    void setIcon(QIcon iconToUse);

private:
    // Child items (links to subdirectories, video clips)
    QList<TreeItem*> m_childItems;
    // The data held by this item (nothing, if it's a directory)
    QList<QVariant> m_itemData;
    // The parent item (e.g. the directory containing the clip/subdirectory represented by this TreeItem)
    TreeItem *m_parentItem;
    // Icon displayed next to the item in the tree viewer
    QIcon icon;
};

#endif // TREEITEM_H
