#include "videodirectorymodel.h"

VideoDirectoryModel::VideoDirectoryModel(std::string path, QObject *parent) : QAbstractItemModel(parent)
{

}

QModelIndex VideoDirectoryModel::index(int row, int column, const QModelIndex &parent) const {
    return QAbstractItemModel::createIndex(0, 0);
}


QModelIndex VideoDirectoryModel::parent(const QModelIndex &index) const {
    return QAbstractItemModel::createIndex(0, 0);
}


int VideoDirectoryModel::rowCount(const QModelIndex &parent) const {
    return 0;
}


int VideoDirectoryModel::columnCount(const QModelIndex &parent) const {
    return 0;
}


QVariant VideoDirectoryModel::data(const QModelIndex &index, int role) const {
    return QVariant(1.0);
}
