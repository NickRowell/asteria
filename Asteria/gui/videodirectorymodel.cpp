#include "videodirectorymodel.h"

#include "util/timeutil.h"

#include <regex>
#include <dirent.h>

VideoDirectoryModel::VideoDirectoryModel(std::string path, QObject *parent) : QAbstractItemModel(parent) {
    QList<QVariant> rootData;
    rootData << "Video clips";
    rootItem = new TreeItem(rootData);
    setupModelData(path, rootItem);
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

    if(role != Qt::DisplayRole) {
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

void VideoDirectoryModel::setupModelData(const std::string &rootPath, TreeItem *parent) {
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

        // Create a TreeItem then search for MM subdirectories
        QList<QVariant> yearData;
        yearData << years[p].c_str();
        TreeItem * yearItem = new TreeItem(yearData, parent);
        yearItem->setIcon(folderIcon);
        parent->appendChild(yearItem);

        // Locate all the subdirectories corresponding to months in the year
        std::string yearPath = rootPath + "/" + years[p];
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

            // Create a TreeItem then search for DD subdirectories
            QList<QVariant> monthData;
            monthData << months[q].c_str();
            TreeItem * monthItem = new TreeItem(monthData, yearItem);
            monthItem->setIcon(folderIcon);
            yearItem->appendChild(monthItem);

            // Locate all the subdirectories corresponding to days in the month
            std::string monthPath = yearPath + "/" + months[q];
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

                // Create a TreeItem then search for subdirectories containing individual clips
                QList<QVariant> dayData;
                dayData << days[r].c_str();
                TreeItem * dayItem = new TreeItem(dayData, monthItem);
                dayItem->setIcon(folderIcon);
                monthItem->appendChild(dayItem);

                // Locate all the subdirectories corresponding to days in the month
                std::string dayPath = monthPath + "/" + days[r];
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

                    // Found a clip directory - create a TreeItem; log the full path to the
                    // clip and use the time as the title for the node.
                    // Set the right kind of icon to use.
                    QList<QVariant> clipData;
                    // Extract time part of UTC string
                    clipData << TimeUtil::extractTimeFromUtcString(clips[s]).c_str();
                    TreeItem * clipItem = new TreeItem(clipData, dayItem);
                    clipItem->setIcon(meteorIcon);
                    dayItem->appendChild(clipItem);

                    std::string clipPath = dayPath + "/" + clips[s];
                }
            } // Close loop over DD
        } // Close loop over MM
    } // Close loop over YYYY

}
