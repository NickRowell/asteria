#include "videodirectorymodel.h"

#include "util/timeutil.h"
#include "gui/treeitemaction.h"
#include "util/fileutil.h"

#include <regex>
#include <dirent.h>

#include <QDebug>

VideoDirectoryModel::VideoDirectoryModel(std::string path, std::string title, QWidget *widget, QObject *parent) : QAbstractItemModel(parent) {
    QList<QVariant> rootData;
    rootData << title.c_str();
    rootItem = new TreeItem(rootData);
    rootPath = path;
    displayWidget = widget;
    setupModelData(path);
}

VideoDirectoryModel::~VideoDirectoryModel() {
    delete rootItem;
}

QModelIndex VideoDirectoryModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItem *parentItem;

    if(!parent.isValid()) {
        parentItem = rootItem;
    }
    else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    TreeItem *childItem = parentItem->child(row);

    if(childItem) {
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}


QModelIndex VideoDirectoryModel::parent(const QModelIndex &index) const {

    if(!index.isValid()) {
        return QModelIndex();
    }

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if(parentItem == rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}


int VideoDirectoryModel::rowCount(const QModelIndex &parent) const {

    if(parent.column() > 0) {
        return 0;
    }

    if(!parent.isValid()) {
        return rootItem->childCount();
    }
    else {
        return static_cast<TreeItem*>(parent.internalPointer())->childCount();
    }
}


int VideoDirectoryModel::columnCount(const QModelIndex &parent) const {
    if(!parent.isValid()) {
        return rootItem->columnCount();
    }
    else {
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    }
}


QVariant VideoDirectoryModel::data(const QModelIndex &index, int role) const {

    if(!index.isValid()) {
        return QVariant();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if(role == Qt::DecorationRole) {
        return item->getIcon();
    }

    if(role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }

    return item->data(index.column());
}

Qt::ItemFlags VideoDirectoryModel::flags(const QModelIndex &index) const {

    if(!index.isValid()) {
        return 0;
    }

    return QAbstractItemModel::flags(index);
}

QVariant VideoDirectoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return rootItem->data(section);
    }

    return QVariant();
}

void VideoDirectoryModel::setupModelData(const std::string &rootPath) {

    // This regex usage relies on version 4.9 or later of the GCC
    const std::regex yearRegex("[0-9]{4}");
    const std::regex monthDayRegex("[0-9]{2}");
    const std::regex utcRegex = TimeUtil::getUtcRegex();

    QIcon folderIcon(":/images/folder-outline-filled.png");
    QIcon meteorIcon(":/images/meteor-512.png");

    // Vector containing names for all YYYY directories found in the root directory, so we can sort them into
    // numerical order for displaying in the tree view
    std::vector<std::string> years;
    DIR *rootDir;
    if ((rootDir = opendir (rootPath.c_str())) != NULL) {
        // Loop over all the files/directories in the root directory
        struct dirent *rootFile;
        while ((rootFile = readdir (rootDir)) != NULL) {
            // Detect directories with names matching the format YYYY
            if(rootFile->d_type == DT_DIR && std::regex_match (rootFile->d_name, yearRegex)) {
                years.push_back(rootFile->d_name);
            }
        }
        closedir (rootDir);
    }

    // Now sort the YYYY directories into ascending numerical order
    std::sort(years.begin(), years.end());

    // Loop over each YYYY directory
    for(unsigned int p=0; p<years.size(); p++) {
        std::string yearPath = rootPath + "/" + years[p];

        // Create a TreeItem then search for MM subdirectories
        QList<QVariant> yearData;
        yearData << years[p].c_str() << yearPath.c_str();
        TreeItem * yearItem = new TreeItem(yearData, rootItem);
        yearItem->setIcon(folderIcon);
        addContextMenu(yearItem);
        rootItem->appendChild(yearItem);

        // Locate all the subdirectories corresponding to months in the year
        std::vector<std::string> months;
        DIR *yearDir;
        if ((yearDir = opendir (yearPath.c_str())) != NULL) {
            struct dirent *yearFile;
            while ((yearFile = readdir (yearDir)) != NULL) {
                // Detect directories with names matching the format MM
                if(yearFile->d_type == DT_DIR && std::regex_match (yearFile->d_name, monthDayRegex)) {
                    months.push_back(yearFile->d_name);
                }
            }
            closedir (yearDir);
        }

        // Now sort the MM directories into ascending numerical order
        std::sort(months.begin(), months.end());

        // Loop over each MM directory
        for(unsigned int q=0; q<months.size(); q++) {
            std::string monthPath = yearPath + "/" + months[q];

            // Create a TreeItem then search for DD subdirectories
            QList<QVariant> monthData;
            monthData << months[q].c_str() << monthPath.c_str();
            TreeItem * monthItem = new TreeItem(monthData, yearItem);
            monthItem->setIcon(folderIcon);
            addContextMenu(monthItem);
            yearItem->appendChild(monthItem);

            // Locate all the subdirectories corresponding to days in the month

            std::vector<std::string> days;
            DIR *monthDir;
            if ((monthDir = opendir (monthPath.c_str())) != NULL) {
                struct dirent *monthFile;
                while ((monthFile = readdir (monthDir)) != NULL) {
                    // Detect directories with names matching the format DD
                    if(monthFile->d_type == DT_DIR && std::regex_match (monthFile->d_name, monthDayRegex)) {
                        days.push_back(monthFile->d_name);
                    }
                }
                closedir (monthDir);
            }

            // Now sort the DD directories into ascending numerical order
            std::sort(days.begin(), days.end());

            // Loop over each DD directory
            for(unsigned int r=0; r<days.size(); r++) {
                std::string dayPath = monthPath + "/" + days[r];

                // Create a TreeItem then search for subdirectories containing individual clips
                QList<QVariant> dayData;
                dayData << days[r].c_str() << dayPath.c_str();
                TreeItem * dayItem = new TreeItem(dayData, monthItem);
                dayItem->setIcon(folderIcon);
                addContextMenu(dayItem);
                monthItem->appendChild(dayItem);

                // Locate all the subdirectories corresponding to days in the month

                std::vector<std::string> clips;
                DIR *dayDir;
                if ((dayDir = opendir (dayPath.c_str())) != NULL) {
                    struct dirent *dayFile;
                    while ((dayFile = readdir (dayDir)) != NULL) {
                        // Detect directories with names matching the UTC format
                        if(dayFile->d_type == DT_DIR && std::regex_match (dayFile->d_name, utcRegex)) {
                            clips.push_back(dayFile->d_name);
                        }
                    }
                    closedir (dayDir);
                }

                // Now sort the clip directories into ascending numerical order
                std::sort(clips.begin(), clips.end());

                // Loop over each clip directory
                for(unsigned int s=0; s<clips.size(); s++) {
                    std::string clipPath = dayPath + "/" + clips[s];
                    // Found a clip directory - create a TreeItem; log the full path to the
                    // clip and use the time as the title for the node.
                    // Set the right kind of icon to use.
                    QList<QVariant> clipData;
                    // Extract time part of UTC string
                    clipData << TimeUtil::extractTimeFromUtcString(clips[s]).c_str() << clipPath.c_str();
                    TreeItem * clipItem = new TreeItem(clipData, dayItem);
                    clipItem->setIcon(meteorIcon);
                    addContextMenu(clipItem);
                    dayItem->appendChild(clipItem);


                }
            } // Close loop over DD
        } // Close loop over MM
    } // Close loop over YYYY

}

/**
 * @brief VideoDirectoryModel::addNewClipByUtc
 * @param utc The UTC string of the first frame in the clip, i.e. that used to determine
 * the path to the directory where the clip results are stored.
 *
 * THINGS THAT HAVEN'T BEEN IMPLEMENTED:
 *  - Adding new clips before the end of the current list, i.e. we assume that new
 *    clips never fall in a month/day/time that does not go at the end of the current
 *    list. This is so we can just append new items to the tree rather than find out
 *    where in the list they should lie
 *  - Removing clips; this may be useful to allow the user to manually delete clips
 *
 */
void VideoDirectoryModel::addNewClipByUtc(std::string utc) {

    qInfo() << "Inserting new clip at " << utc.c_str();

    // Extract YYYY, MM, DD from UTC
    std::string yyyy = TimeUtil::extractYearFromUtcString(utc);
    std::string mm = TimeUtil::extractMonthFromUtcString(utc);
    std::string dd = TimeUtil::extractDayFromUtcString(utc);

    QIcon folderIcon(":/images/folder-outline-filled.png");
    QIcon meteorIcon(":/images/meteor-512.png");

    // Build the full path to the clip
    std::string yearPath = rootPath + "/" + yyyy;
    std::string monthPath = yearPath + "/" + mm;
    std::string dayPath = monthPath  + "/" + dd;
    std::string clipPath = dayPath + "/" + utc;

    // Check if YYYY exists in the root directory
    unsigned int years = rootItem->childCount();
    TreeItem * clipYearItem = NULL;
    for(unsigned int year = 0; year < years; year++) {
        TreeItem * existingYearItem = rootItem->child(year);
        // Check if the directory represented by this TreeItem corresponds to the year of the new clip
        QVariant yearDirPath = existingYearItem->data(0);
        if(yyyy.compare(yearDirPath.toString().toStdString())==0) {
            // Found existing YYYY directory
            clipYearItem = existingYearItem;
            break;
        }
    }

    if(!clipYearItem) {
        // No existing year item - create it. Note that it's OK to append this to the end
        // of the existing items, because new years will always be after existing years to
        // the ascending order of the items in the display is preserved.
        QList<QVariant> yearData;
        yearData << yyyy.c_str() << yearPath.c_str();
        clipYearItem = new TreeItem(yearData, rootItem);
        clipYearItem->setIcon(folderIcon);
        addContextMenu(clipYearItem);

        int existingRows = rootItem->childCount();
        QAbstractItemModel::beginInsertRows(QModelIndex(), existingRows, existingRows);
        rootItem->appendChild(clipYearItem);
        QAbstractItemModel::endInsertRows();
    }

    QModelIndex yearIdx = index(clipYearItem->row(), 0, QModelIndex());

    // Check if MM exists in the year item
    unsigned int months = clipYearItem->childCount();
    TreeItem * clipMonthItem = NULL;

    for(unsigned int month = 0; month < months; month++) {
        TreeItem * existingMonthItem = clipYearItem->child(month);
        QVariant monthDirPath = existingMonthItem->data(0);
        if(mm.compare(monthDirPath.toString().toStdString())==0) {
            clipMonthItem = existingMonthItem;
            break;
        }
    }
    if(!clipMonthItem) {
        // Create new MM item
        QList<QVariant> monthData;
        monthData << mm.c_str() << monthPath.c_str();
        clipMonthItem = new TreeItem(monthData, clipYearItem);
        clipMonthItem->setIcon(folderIcon);
        addContextMenu(clipMonthItem);

        int existingRows = clipYearItem->childCount();
        QAbstractItemModel::beginInsertRows(yearIdx, existingRows, existingRows);
        clipYearItem->appendChild(clipMonthItem);
        QAbstractItemModel::endInsertRows();
    }

    QModelIndex monthIdx = index(clipMonthItem->row(), 0, yearIdx);

    // Check if DD exists in the month item
    unsigned int days = clipMonthItem->childCount();
    TreeItem * clipDayItem = NULL;
    for(unsigned int day = 0; day < days; day++) {
        TreeItem * existingDayItem = clipMonthItem->child(day);
        QVariant dayDirPath = existingDayItem->data(0);
        if(dd.compare(dayDirPath.toString().toStdString())==0) {
            clipDayItem = existingDayItem;
            break;
        }
    }
    if(!clipDayItem) {
        // Create new DD item
        QList<QVariant> dayData;
        dayData << dd.c_str() << dayPath.c_str();
        clipDayItem = new TreeItem(dayData, clipMonthItem);
        clipDayItem->setIcon(folderIcon);
        addContextMenu(clipDayItem);

        int existingRows = clipMonthItem->childCount();
        QAbstractItemModel::beginInsertRows(monthIdx, existingRows, existingRows);
        clipMonthItem->appendChild(clipDayItem);
        QAbstractItemModel::endInsertRows();
    }

    QModelIndex dayIdx = index(clipDayItem->row(), 0, monthIdx);

    // Add new clip
    QList<QVariant> clipData;
    clipData << TimeUtil::extractTimeFromUtcString(utc).c_str() << clipPath.c_str();
    TreeItem * clipItem = new TreeItem(clipData, clipDayItem);
    clipItem->setIcon(meteorIcon);
    addContextMenu(clipItem);

    int existingRows = clipDayItem->childCount();
    QAbstractItemModel::beginInsertRows(dayIdx, existingRows, existingRows);
    clipDayItem->appendChild(clipItem);
    QAbstractItemModel::endInsertRows();
}

void VideoDirectoryModel::addContextMenu(TreeItem * item) {
    QMenu * contextMenu = new QMenu(QString("Title"), displayWidget);
    TreeItemAction * deleteAction = new TreeItemAction("Delete", item, contextMenu);
    contextMenu->addAction(deleteAction);
    connect(deleteAction, SIGNAL (itemClicked(TreeItem *)), this, SLOT( deleteItem(TreeItem *)));
    item->setContextMenu(contextMenu);
}

void VideoDirectoryModel::deleteItem(TreeItem * itemToDelete) {

    std::string pathToItem = itemToDelete->data(1).toString().toStdString();

    // Delete the files from disk
    FileUtil::deleteFilePath(pathToItem);

    // Recurse through the tree deleting each child item
    removeTreeItemsRecursive(itemToDelete);
}

void VideoDirectoryModel::removeTreeItemsRecursive(TreeItem * itemToDelete) {

    // Delete each of the item's children
    for(unsigned int child = 0u; child < itemToDelete->childCount(); child++) {
        removeTreeItemsRecursive(itemToDelete->child(child));
    }
    // Now delete this item (remove it from it's parent's list)

    // Get the model index of the parent of the item to be deleted
    QModelIndex parentIdx = createIndex(itemToDelete->row(), 0, itemToDelete->parentItem());

    QAbstractItemModel::beginRemoveRows(parentIdx, itemToDelete->row(), itemToDelete->row());
    itemToDelete->parentItem()->removeChild(itemToDelete->row());
    QAbstractItemModel::endRemoveRows();
}
