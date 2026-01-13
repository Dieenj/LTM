/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_MainWindow_t {
    uint offsetsAndSizes[136];
    char stringdata0[11];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[21];
    char stringdata5[19];
    char stringdata6[22];
    char stringdata7[4];
    char stringdata8[21];
    char stringdata9[17];
    char stringdata10[16];
    char stringdata11[20];
    char stringdata12[22];
    char stringdata13[18];
    char stringdata14[24];
    char stringdata15[15];
    char stringdata16[16];
    char stringdata17[16];
    char stringdata18[16];
    char stringdata19[13];
    char stringdata20[6];
    char stringdata21[16];
    char stringdata22[4];
    char stringdata23[22];
    char stringdata24[4];
    char stringdata25[7];
    char stringdata26[20];
    char stringdata27[20];
    char stringdata28[27];
    char stringdata29[25];
    char stringdata30[21];
    char stringdata31[25];
    char stringdata32[25];
    char stringdata33[25];
    char stringdata34[8];
    char stringdata35[5];
    char stringdata36[24];
    char stringdata37[8];
    char stringdata38[9];
    char stringdata39[10];
    char stringdata40[6];
    char stringdata41[23];
    char stringdata42[23];
    char stringdata43[7];
    char stringdata44[19];
    char stringdata45[27];
    char stringdata46[27];
    char stringdata47[6];
    char stringdata48[23];
    char stringdata49[21];
    char stringdata50[18];
    char stringdata51[23];
    char stringdata52[25];
    char stringdata53[24];
    char stringdata54[5];
    char stringdata55[22];
    char stringdata56[21];
    char stringdata57[6];
    char stringdata58[15];
    char stringdata59[5];
    char stringdata60[20];
    char stringdata61[21];
    char stringdata62[22];
    char stringdata63[23];
    char stringdata64[18];
    char stringdata65[19];
    char stringdata66[19];
    char stringdata67[13];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 19),  // "onConnectBtnClicked"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 17),  // "onLoginBtnClicked"
        QT_MOC_LITERAL(50, 20),  // "onRegisterBtnClicked"
        QT_MOC_LITERAL(71, 18),  // "handleLoginSuccess"
        QT_MOC_LITERAL(90, 21),  // "handleRegisterSuccess"
        QT_MOC_LITERAL(112, 3),  // "msg"
        QT_MOC_LITERAL(116, 20),  // "handleRegisterFailed"
        QT_MOC_LITERAL(137, 16),  // "onRefreshClicked"
        QT_MOC_LITERAL(154, 15),  // "onUploadClicked"
        QT_MOC_LITERAL(170, 19),  // "onUploadFileClicked"
        QT_MOC_LITERAL(190, 21),  // "onUploadFolderClicked"
        QT_MOC_LITERAL(212, 17),  // "onDownloadClicked"
        QT_MOC_LITERAL(230, 23),  // "onDownloadFolderClicked"
        QT_MOC_LITERAL(254, 14),  // "onShareClicked"
        QT_MOC_LITERAL(269, 15),  // "onDeleteClicked"
        QT_MOC_LITERAL(285, 15),  // "onRenameClicked"
        QT_MOC_LITERAL(301, 15),  // "onLogoutClicked"
        QT_MOC_LITERAL(317, 12),  // "onTabChanged"
        QT_MOC_LITERAL(330, 5),  // "index"
        QT_MOC_LITERAL(336, 15),  // "showContextMenu"
        QT_MOC_LITERAL(352, 3),  // "pos"
        QT_MOC_LITERAL(356, 21),  // "onFolderDoubleClicked"
        QT_MOC_LITERAL(378, 3),  // "row"
        QT_MOC_LITERAL(382, 6),  // "column"
        QT_MOC_LITERAL(389, 19),  // "onBackButtonClicked"
        QT_MOC_LITERAL(409, 19),  // "onBreadcrumbClicked"
        QT_MOC_LITERAL(429, 26),  // "onGenerateShareCodeClicked"
        QT_MOC_LITERAL(456, 24),  // "onRedeemShareCodeClicked"
        QT_MOC_LITERAL(481, 20),  // "onRevokeShareClicked"
        QT_MOC_LITERAL(502, 24),  // "onDeleteShareCodeClicked"
        QT_MOC_LITERAL(527, 24),  // "onRefreshMySharesClicked"
        QT_MOC_LITERAL(552, 24),  // "handleShareCodeGenerated"
        QT_MOC_LITERAL(577, 7),  // "success"
        QT_MOC_LITERAL(585, 4),  // "code"
        QT_MOC_LITERAL(590, 23),  // "handleShareCodeRedeemed"
        QT_MOC_LITERAL(614, 7),  // "file_id"
        QT_MOC_LITERAL(622, 8),  // "filename"
        QT_MOC_LITERAL(631, 9),  // "is_folder"
        QT_MOC_LITERAL(641, 5),  // "owner"
        QT_MOC_LITERAL(647, 22),  // "handleMySharesReceived"
        QT_MOC_LITERAL(670, 22),  // "QList<ShareInfoClient>"
        QT_MOC_LITERAL(693, 6),  // "shares"
        QT_MOC_LITERAL(700, 18),  // "handleShareRevoked"
        QT_MOC_LITERAL(719, 26),  // "handleMyShareCodesReceived"
        QT_MOC_LITERAL(746, 26),  // "QList<ShareCodeInfoClient>"
        QT_MOC_LITERAL(773, 5),  // "codes"
        QT_MOC_LITERAL(779, 22),  // "handleShareCodeDeleted"
        QT_MOC_LITERAL(802, 20),  // "onGuestAccessClicked"
        QT_MOC_LITERAL(823, 17),  // "onGuestRedeemCode"
        QT_MOC_LITERAL(841, 22),  // "onGuestDownloadClicked"
        QT_MOC_LITERAL(864, 24),  // "onGuestFileDoubleClicked"
        QT_MOC_LITERAL(889, 23),  // "handleGuestRedeemResult"
        QT_MOC_LITERAL(913, 4),  // "size"
        QT_MOC_LITERAL(918, 21),  // "handleGuestFolderList"
        QT_MOC_LITERAL(940, 20),  // "QList<GuestFileInfo>"
        QT_MOC_LITERAL(961, 5),  // "files"
        QT_MOC_LITERAL(967, 14),  // "handleFileList"
        QT_MOC_LITERAL(982, 4),  // "data"
        QT_MOC_LITERAL(987, 19),  // "handleUploadStarted"
        QT_MOC_LITERAL(1007, 20),  // "handleUploadProgress"
        QT_MOC_LITERAL(1028, 21),  // "handleDownloadStarted"
        QT_MOC_LITERAL(1050, 22),  // "handleDownloadComplete"
        QT_MOC_LITERAL(1073, 17),  // "handleShareResult"
        QT_MOC_LITERAL(1091, 18),  // "handleDeleteResult"
        QT_MOC_LITERAL(1110, 18),  // "handleRenameResult"
        QT_MOC_LITERAL(1129, 12)   // "handleLogout"
    },
    "MainWindow",
    "onConnectBtnClicked",
    "",
    "onLoginBtnClicked",
    "onRegisterBtnClicked",
    "handleLoginSuccess",
    "handleRegisterSuccess",
    "msg",
    "handleRegisterFailed",
    "onRefreshClicked",
    "onUploadClicked",
    "onUploadFileClicked",
    "onUploadFolderClicked",
    "onDownloadClicked",
    "onDownloadFolderClicked",
    "onShareClicked",
    "onDeleteClicked",
    "onRenameClicked",
    "onLogoutClicked",
    "onTabChanged",
    "index",
    "showContextMenu",
    "pos",
    "onFolderDoubleClicked",
    "row",
    "column",
    "onBackButtonClicked",
    "onBreadcrumbClicked",
    "onGenerateShareCodeClicked",
    "onRedeemShareCodeClicked",
    "onRevokeShareClicked",
    "onDeleteShareCodeClicked",
    "onRefreshMySharesClicked",
    "handleShareCodeGenerated",
    "success",
    "code",
    "handleShareCodeRedeemed",
    "file_id",
    "filename",
    "is_folder",
    "owner",
    "handleMySharesReceived",
    "QList<ShareInfoClient>",
    "shares",
    "handleShareRevoked",
    "handleMyShareCodesReceived",
    "QList<ShareCodeInfoClient>",
    "codes",
    "handleShareCodeDeleted",
    "onGuestAccessClicked",
    "onGuestRedeemCode",
    "onGuestDownloadClicked",
    "onGuestFileDoubleClicked",
    "handleGuestRedeemResult",
    "size",
    "handleGuestFolderList",
    "QList<GuestFileInfo>",
    "files",
    "handleFileList",
    "data",
    "handleUploadStarted",
    "handleUploadProgress",
    "handleDownloadStarted",
    "handleDownloadComplete",
    "handleShareResult",
    "handleDeleteResult",
    "handleRenameResult",
    "handleLogout"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      47,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  296,    2, 0x08,    1 /* Private */,
       3,    0,  297,    2, 0x08,    2 /* Private */,
       4,    0,  298,    2, 0x08,    3 /* Private */,
       5,    0,  299,    2, 0x08,    4 /* Private */,
       6,    1,  300,    2, 0x08,    5 /* Private */,
       8,    1,  303,    2, 0x08,    7 /* Private */,
       9,    0,  306,    2, 0x08,    9 /* Private */,
      10,    0,  307,    2, 0x08,   10 /* Private */,
      11,    0,  308,    2, 0x08,   11 /* Private */,
      12,    0,  309,    2, 0x08,   12 /* Private */,
      13,    0,  310,    2, 0x08,   13 /* Private */,
      14,    0,  311,    2, 0x08,   14 /* Private */,
      15,    0,  312,    2, 0x08,   15 /* Private */,
      16,    0,  313,    2, 0x08,   16 /* Private */,
      17,    0,  314,    2, 0x08,   17 /* Private */,
      18,    0,  315,    2, 0x08,   18 /* Private */,
      19,    1,  316,    2, 0x08,   19 /* Private */,
      21,    1,  319,    2, 0x08,   21 /* Private */,
      23,    2,  322,    2, 0x08,   23 /* Private */,
      26,    0,  327,    2, 0x08,   26 /* Private */,
      27,    0,  328,    2, 0x08,   27 /* Private */,
      28,    0,  329,    2, 0x08,   28 /* Private */,
      29,    0,  330,    2, 0x08,   29 /* Private */,
      30,    0,  331,    2, 0x08,   30 /* Private */,
      31,    0,  332,    2, 0x08,   31 /* Private */,
      32,    0,  333,    2, 0x08,   32 /* Private */,
      33,    3,  334,    2, 0x08,   33 /* Private */,
      36,    5,  341,    2, 0x08,   37 /* Private */,
      41,    1,  352,    2, 0x08,   43 /* Private */,
      44,    2,  355,    2, 0x08,   45 /* Private */,
      45,    1,  360,    2, 0x08,   48 /* Private */,
      48,    2,  363,    2, 0x08,   50 /* Private */,
      49,    0,  368,    2, 0x08,   53 /* Private */,
      50,    0,  369,    2, 0x08,   54 /* Private */,
      51,    0,  370,    2, 0x08,   55 /* Private */,
      52,    2,  371,    2, 0x08,   56 /* Private */,
      53,    6,  376,    2, 0x08,   59 /* Private */,
      55,    1,  389,    2, 0x08,   66 /* Private */,
      58,    1,  392,    2, 0x08,   68 /* Private */,
      60,    1,  395,    2, 0x08,   70 /* Private */,
      61,    1,  398,    2, 0x08,   72 /* Private */,
      62,    1,  401,    2, 0x08,   74 /* Private */,
      63,    1,  404,    2, 0x08,   76 /* Private */,
      64,    2,  407,    2, 0x08,   78 /* Private */,
      65,    2,  412,    2, 0x08,   81 /* Private */,
      66,    2,  417,    2, 0x08,   84 /* Private */,
      67,    0,  422,    2, 0x08,   87 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   20,
    QMetaType::Void, QMetaType::QPoint,   22,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString, QMetaType::QString,   34,   35,    7,
    QMetaType::Void, QMetaType::Bool, QMetaType::LongLong, QMetaType::QString, QMetaType::Bool, QMetaType::QString,   34,   37,   38,   39,   40,
    QMetaType::Void, 0x80000000 | 42,   43,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   34,    7,
    QMetaType::Void, 0x80000000 | 46,   47,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   34,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   24,   25,
    QMetaType::Void, QMetaType::Bool, QMetaType::LongLong, QMetaType::QString, QMetaType::Bool, QMetaType::QString, QMetaType::LongLong,   34,   37,   38,   39,   40,   54,
    QMetaType::Void, 0x80000000 | 56,   57,
    QMetaType::Void, QMetaType::QString,   59,
    QMetaType::Void, QMetaType::QString,   38,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,   38,
    QMetaType::Void, QMetaType::QString,   38,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   34,    7,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   34,    7,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   34,    7,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSizes,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'onConnectBtnClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLoginBtnClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRegisterBtnClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleLoginSuccess'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleRegisterSuccess'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleRegisterFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'onRefreshClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onUploadClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onUploadFileClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onUploadFolderClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDownloadClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDownloadFolderClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onShareClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeleteClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRenameClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLogoutClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'showContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'onFolderDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onBackButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBreadcrumbClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onGenerateShareCodeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRedeemShareCodeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRevokeShareClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeleteShareCodeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRefreshMySharesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleShareCodeGenerated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'handleShareCodeRedeemed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<long long, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'handleMySharesReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<ShareInfoClient> &, std::false_type>,
        // method 'handleShareRevoked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'handleMyShareCodesReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<ShareCodeInfoClient> &, std::false_type>,
        // method 'handleShareCodeDeleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onGuestAccessClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onGuestRedeemCode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onGuestDownloadClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onGuestFileDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleGuestRedeemResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<long long, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<long long, std::false_type>,
        // method 'handleGuestFolderList'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<GuestFileInfo> &, std::false_type>,
        // method 'handleFileList'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleUploadStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleUploadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleDownloadStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleDownloadComplete'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleShareResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleDeleteResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleRenameResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        // method 'handleLogout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onConnectBtnClicked(); break;
        case 1: _t->onLoginBtnClicked(); break;
        case 2: _t->onRegisterBtnClicked(); break;
        case 3: _t->handleLoginSuccess(); break;
        case 4: _t->handleRegisterSuccess((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->handleRegisterFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->onRefreshClicked(); break;
        case 7: _t->onUploadClicked(); break;
        case 8: _t->onUploadFileClicked(); break;
        case 9: _t->onUploadFolderClicked(); break;
        case 10: _t->onDownloadClicked(); break;
        case 11: _t->onDownloadFolderClicked(); break;
        case 12: _t->onShareClicked(); break;
        case 13: _t->onDeleteClicked(); break;
        case 14: _t->onRenameClicked(); break;
        case 15: _t->onLogoutClicked(); break;
        case 16: _t->onTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->showContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 18: _t->onFolderDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->onBackButtonClicked(); break;
        case 20: _t->onBreadcrumbClicked(); break;
        case 21: _t->onGenerateShareCodeClicked(); break;
        case 22: _t->onRedeemShareCodeClicked(); break;
        case 23: _t->onRevokeShareClicked(); break;
        case 24: _t->onDeleteShareCodeClicked(); break;
        case 25: _t->onRefreshMySharesClicked(); break;
        case 26: _t->handleShareCodeGenerated((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 27: _t->handleShareCodeRedeemed((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[5]))); break;
        case 28: _t->handleMySharesReceived((*reinterpret_cast< std::add_pointer_t<QList<ShareInfoClient>>>(_a[1]))); break;
        case 29: _t->handleShareRevoked((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 30: _t->handleMyShareCodesReceived((*reinterpret_cast< std::add_pointer_t<QList<ShareCodeInfoClient>>>(_a[1]))); break;
        case 31: _t->handleShareCodeDeleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 32: _t->onGuestAccessClicked(); break;
        case 33: _t->onGuestRedeemCode(); break;
        case 34: _t->onGuestDownloadClicked(); break;
        case 35: _t->onGuestFileDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 36: _t->handleGuestRedeemResult((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[6]))); break;
        case 37: _t->handleGuestFolderList((*reinterpret_cast< std::add_pointer_t<QList<GuestFileInfo>>>(_a[1]))); break;
        case 38: _t->handleFileList((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 39: _t->handleUploadStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 40: _t->handleUploadProgress((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 41: _t->handleDownloadStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 42: _t->handleDownloadComplete((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 43: _t->handleShareResult((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 44: _t->handleDeleteResult((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 45: _t->handleRenameResult((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 46: _t->handleLogout(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 47)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 47;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 47)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 47;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
