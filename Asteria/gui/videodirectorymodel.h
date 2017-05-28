#ifndef VIDEODIRECTORYMODEL_H
#define VIDEODIRECTORYMODEL_H

#include <QAbstractItemModel>

/**
 * @brief The VideoDirectoryModel class
 * Provides a model for the video directory tree that can be used to navigate
 * the videos and analysis results in a tree view.
 */
class VideoDirectoryModel : public QAbstractItemModel
{
public:
    /**
     * @brief VideoDirectoryModel
     * @param path The path to the video directory
     */
    VideoDirectoryModel(std::string path, QObject *parent = 0);


    // According to http://doc.qt.io/qt-5/qabstractitemmodel.html#details this is the minimum
    // set of functions that have to be implemented:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

};

#endif // VIDEODIRECTORYMODEL_H
