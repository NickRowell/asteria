/****************************************************************************
** Meta object code from reading C++ file 'acquisitionthread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Asteria/infra/acquisitionthread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'acquisitionthread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_AcquisitionThread_t {
    QByteArrayData data[14];
    char stringdata0[178];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AcquisitionThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AcquisitionThread_t qt_meta_stringdata_AcquisitionThread = {
    {
QT_MOC_LITERAL(0, 0, 17), // "AcquisitionThread"
QT_MOC_LITERAL(1, 18, 13), // "acquiredImage"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 22), // "std::shared_ptr<Image>"
QT_MOC_LITERAL(4, 56, 12), // "acquiredClip"
QT_MOC_LITERAL(5, 69, 11), // "std::string"
QT_MOC_LITERAL(6, 81, 3), // "utc"
QT_MOC_LITERAL(7, 85, 19), // "transitionedToState"
QT_MOC_LITERAL(8, 105, 35), // "AcquisitionThread::Acquisitio..."
QT_MOC_LITERAL(9, 141, 6), // "launch"
QT_MOC_LITERAL(10, 148, 8), // "shutdown"
QT_MOC_LITERAL(11, 157, 7), // "preview"
QT_MOC_LITERAL(12, 165, 5), // "pause"
QT_MOC_LITERAL(13, 171, 6) // "detect"

    },
    "AcquisitionThread\0acquiredImage\0\0"
    "std::shared_ptr<Image>\0acquiredClip\0"
    "std::string\0utc\0transitionedToState\0"
    "AcquisitionThread::AcquisitionState\0"
    "launch\0shutdown\0preview\0pause\0detect"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AcquisitionThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    1,   57,    2, 0x06 /* Public */,
       7,    1,   60,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   63,    2, 0x0a /* Public */,
      10,    0,   64,    2, 0x0a /* Public */,
      11,    0,   65,    2, 0x0a /* Public */,
      12,    0,   66,    2, 0x0a /* Public */,
      13,    0,   67,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 8,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AcquisitionThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        AcquisitionThread *_t = static_cast<AcquisitionThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->acquiredImage((*reinterpret_cast< std::shared_ptr<Image>(*)>(_a[1]))); break;
        case 1: _t->acquiredClip((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 2: _t->transitionedToState((*reinterpret_cast< AcquisitionThread::AcquisitionState(*)>(_a[1]))); break;
        case 3: _t->launch(); break;
        case 4: _t->shutdown(); break;
        case 5: _t->preview(); break;
        case 6: _t->pause(); break;
        case 7: _t->detect(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (AcquisitionThread::*_t)(std::shared_ptr<Image> );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AcquisitionThread::acquiredImage)) {
                *result = 0;
            }
        }
        {
            typedef void (AcquisitionThread::*_t)(std::string );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AcquisitionThread::acquiredClip)) {
                *result = 1;
            }
        }
        {
            typedef void (AcquisitionThread::*_t)(AcquisitionThread::AcquisitionState );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AcquisitionThread::transitionedToState)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject AcquisitionThread::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_AcquisitionThread.data,
      qt_meta_data_AcquisitionThread,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *AcquisitionThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AcquisitionThread::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_AcquisitionThread.stringdata0))
        return static_cast<void*>(const_cast< AcquisitionThread*>(this));
    return QThread::qt_metacast(_clname);
}

int AcquisitionThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void AcquisitionThread::acquiredImage(std::shared_ptr<Image> _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AcquisitionThread::acquiredClip(std::string _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AcquisitionThread::transitionedToState(AcquisitionThread::AcquisitionState _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
