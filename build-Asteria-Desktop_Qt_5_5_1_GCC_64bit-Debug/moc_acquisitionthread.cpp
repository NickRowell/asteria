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
    QByteArrayData data[16];
    char stringdata0[200];
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
QT_MOC_LITERAL(4, 56, 10), // "videoStats"
QT_MOC_LITERAL(5, 67, 10), // "VideoStats"
QT_MOC_LITERAL(6, 78, 12), // "acquiredClip"
QT_MOC_LITERAL(7, 91, 11), // "std::string"
QT_MOC_LITERAL(8, 103, 3), // "utc"
QT_MOC_LITERAL(9, 107, 19), // "transitionedToState"
QT_MOC_LITERAL(10, 127, 35), // "AcquisitionThread::Acquisitio..."
QT_MOC_LITERAL(11, 163, 6), // "launch"
QT_MOC_LITERAL(12, 170, 8), // "shutdown"
QT_MOC_LITERAL(13, 179, 7), // "preview"
QT_MOC_LITERAL(14, 187, 5), // "pause"
QT_MOC_LITERAL(15, 193, 6) // "detect"

    },
    "AcquisitionThread\0acquiredImage\0\0"
    "std::shared_ptr<Image>\0videoStats\0"
    "VideoStats\0acquiredClip\0std::string\0"
    "utc\0transitionedToState\0"
    "AcquisitionThread::AcquisitionState\0"
    "launch\0shutdown\0preview\0pause\0detect"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AcquisitionThread[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    1,   62,    2, 0x06 /* Public */,
       6,    1,   65,    2, 0x06 /* Public */,
       9,    1,   68,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   71,    2, 0x0a /* Public */,
      12,    0,   72,    2, 0x0a /* Public */,
      13,    0,   73,    2, 0x0a /* Public */,
      14,    0,   74,    2, 0x0a /* Public */,
      15,    0,   75,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 10,    2,

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
        case 1: _t->videoStats((*reinterpret_cast< const VideoStats(*)>(_a[1]))); break;
        case 2: _t->acquiredClip((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 3: _t->transitionedToState((*reinterpret_cast< AcquisitionThread::AcquisitionState(*)>(_a[1]))); break;
        case 4: _t->launch(); break;
        case 5: _t->shutdown(); break;
        case 6: _t->preview(); break;
        case 7: _t->pause(); break;
        case 8: _t->detect(); break;
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
            typedef void (AcquisitionThread::*_t)(const VideoStats & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AcquisitionThread::videoStats)) {
                *result = 1;
            }
        }
        {
            typedef void (AcquisitionThread::*_t)(std::string );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AcquisitionThread::acquiredClip)) {
                *result = 2;
            }
        }
        {
            typedef void (AcquisitionThread::*_t)(AcquisitionThread::AcquisitionState );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&AcquisitionThread::transitionedToState)) {
                *result = 3;
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
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
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
void AcquisitionThread::videoStats(const VideoStats & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AcquisitionThread::acquiredClip(std::string _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AcquisitionThread::transitionedToState(AcquisitionThread::AcquisitionState _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
