#ifndef VIDEODIRECTORYMODEL_H
#define VIDEODIRECTORYMODEL_H

#include "gui/treeitem.h"

#include <string>

#include <QAbstractItemModel>

/**
 * @brief The VideoDirectoryModel class
 * Provides a model for the video directory tree that can be used to navigate
 * the videos and analysis results in a tree view.
 *
 * Follows the example at http://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
 */
class VideoDirectoryModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /**
     * @brief VideoDirectoryModel
     * @param path The path to the video directory
     */
    explicit VideoDirectoryModel(std::string path, QObject *parent = 0);
    ~VideoDirectoryModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:

    void setupModelData(const std::string &rootPath, TreeItem *parent);

    TreeItem *rootItem;
};

#endif // VIDEODIRECTORYMODEL_H
