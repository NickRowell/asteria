/****************************************************************************
** Meta object code from reading C++ file 'configcreationwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Asteria/gui/configcreationwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'configcreationwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ConfigCreationWindow_t {
    QByteArrayData data[8];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ConfigCreationWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ConfigCreationWindow_t qt_meta_stringdata_ConfigCreationWindow = {
    {
QT_MOC_LITERAL(0, 0, 20), // "ConfigCreationWindow"
QT_MOC_LITERAL(1, 21, 2), // "ok"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 6), // "cancel"
QT_MOC_LITERAL(4, 32, 11), // "loadClicked"
QT_MOC_LITERAL(5, 44, 11), // "saveClicked"
QT_MOC_LITERAL(6, 56, 9), // "okClicked"
QT_MOC_LITERAL(7, 66, 13) // "cancelClicked"

    },
    "ConfigCreationWindow\0ok\0\0cancel\0"
    "loadClicked\0saveClicked\0okClicked\0"
    "cancelClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ConfigCreationWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   46,    2, 0x0a /* Public */,
       5,    0,   47,    2, 0x0a /* Public */,
       6,    0,   48,    2, 0x0a /* Public */,
       7,    0,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ConfigCreationWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ConfigCreationWindow *_t = static_cast<ConfigCreationWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ok(); break;
        case 1: _t->cancel(); break;
        case 2: _t->loadClicked(); break;
        case 3: _t->saveClicked(); break;
        case 4: _t->okClicked(); break;
        case 5: _t->cancelClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ConfigCreationWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ConfigCreationWindow::ok)) {
                *result = 0;
            }
        }
        {
            typedef void (ConfigCreationWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ConfigCreationWindow::cancel)) {
                *result = 1;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject ConfigCreationWindow::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ConfigCreationWindow.data,
      qt_meta_data_ConfigCreationWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ConfigCreationWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConfigCreationWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ConfigCreationWindow.stringdata0))
        return static_cast<void*>(const_cast< ConfigCreationWindow*>(this));
    return QDialog::qt_metacast(_clname);
}

int ConfigCreationWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ConfigCreationWindow::ok()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void ConfigCreationWindow::cancel()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
