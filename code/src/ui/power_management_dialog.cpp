
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/power_management_dialog.h"
#include "onyx/ui/keyboard_navigator.h"
#include "onyx/data/data_tags.h"
#include "onyx/screen/screen_update_watcher.h"

namespace ui
{

static const char* SCOPE = "pm";
static const int ITEM_HEIGHT = 80;
static const QString TITLE_INDEX = "title_index";

struct ItemStruct
{
    const char * title;
    int standby_seconds;
    int shutdown_seconds;
};

/// Define all display items.
static const ItemStruct DISPLAY_ITEMS[] =
{
    {QT_TRANSLATE_NOOP("pm","3 minutes to standby"), 180 * 1000, 0},
    {QT_TRANSLATE_NOOP("pm","5 minutes to shutdown"), 0, 300 * 1000},
    {QT_TRANSLATE_NOOP("pm","3 minutes to standby / 5 minutes to shutdown"), 180 * 1000, 300 * 1000},
    {QT_TRANSLATE_NOOP("pm","10 minutes to standby / 15 minutes to shutdown"), 600 * 1000, 900 * 1000},
};

static const int DISPLAY_COUNT = sizeof(DISPLAY_ITEMS) / sizeof(DISPLAY_ITEMS[0]);

static bool isPmExclusive()
{
    return (qgetenv("PM_INCLUSIVE").toInt() <= 0);
}

PowerManagementDialog::PowerManagementDialog(QWidget *parent, SysStatus & ref)
    : OnyxDialog(parent)
    , status_(ref)
    , ver_layout_(&content_widget_)
    , hor_layout_(0)
    , sys_standby_interval_(0)
    , standby_interval_(0)
    , sys_shutdown_interval_(0)
    , shutdown_interval_(0)
{
    setModal(true);
    if(isPmExclusive())
    {
        resize(400, 300);
    }
    else
    {
        resize(400, 500);
    }
    interval_selected_ = NULL;
    createLayout();
    onyx::screen::watcher().addWatcher(this);
}

PowerManagementDialog::~PowerManagementDialog(void)
{
}

int PowerManagementDialog::exec()
{
    shadows_.show(true);
    show();
    if(interval_selected_)
    {
        buttons_.setFocus();
        buttons_.setFocusTo(0, interval_selected_->value(TITLE_INDEX).toInt());
    }
    else
    {
        ok_.setFocus();
        ok_.setFocusTo(0, 0);
    }
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(
        0,
        outbounding(parentWidget()),
        onyx::screen::ScreenProxy::GC,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void PowerManagementDialog::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void PowerManagementDialog::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    switch (ke->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
        break;
    case Qt::Key_Return:
        break;
    case Qt::Key_Escape:
        reject();
        onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
        break;
    }
}

void PowerManagementDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);

    // Retrieve the values from system status.
    sys_standby_interval_ = status_.suspendInterval();
    standby_interval_ = sys_standby_interval_;

    sys_shutdown_interval_ = status_.shutdownInterval();
    shutdown_interval_ = sys_shutdown_interval_;

    updateTitle(QApplication::tr("Power Management"));
    updateTitleIcon(QPixmap(":/images/power.png"));

    // The big layout.
    ver_layout_.setContentsMargins(SPACING, 0, SPACING, 0);
    ver_layout_.addSpacing(10);

    // Create display items
    buttons_.setSubItemType(CheckBoxView::type());
    buttons_.setMargin(2, 2, 2, 2);
    buttons_.setPreferItemSize(QSize(0, ITEM_HEIGHT));
    ODatas d;

    int display_cout = isPmExclusive() ? 2: DISPLAY_COUNT;
    for (int row = 0; row < display_cout; ++row)
    {
        OData * item = new OData;
        item->insert(TAG_TITLE, qApp->translate(SCOPE, DISPLAY_ITEMS[row].title));
        item->insert(TITLE_INDEX, row);
        if ( (sys_standby_interval_ == DISPLAY_ITEMS[row].standby_seconds) &&
             (sys_shutdown_interval_ == DISPLAY_ITEMS[row].shutdown_seconds)
           )
        {
            interval_selected_ = item;
            item->insert(TAG_CHECKED, true);
        }
        d.push_back(item);
    }

    buttons_.setData(d, true);
    buttons_.setMinimumHeight( (ITEM_HEIGHT+2)*d.size());
    buttons_.setFixedGrid(d.size(), 1);
    buttons_.setSpacing(3);
    QObject::connect(&buttons_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)),
                     this, SLOT(onButtonChanged(CatalogView *, ContentView *, int)), Qt::QueuedConnection);

    ver_layout_.addWidget(&buttons_);


    // OK cancel buttons.
    ok_.setSubItemType(ui::CoverView::type());
    ok_.setPreferItemSize(QSize(100, 60));
    ODatas d2;

    OData * item = new OData;
    item->insert(TAG_TITLE, tr("OK"));
    item->insert(TITLE_INDEX, 1);
    d2.push_back(item);


    ok_.setData(d2, true);
    ok_.setMinimumHeight( 60 );
    ok_.setMinimumWidth(100);
    ok_.setFocusPolicy(Qt::TabFocus);
    ok_.setNeighbor(&buttons_, CatalogView::UP);
    connect(&ok_, SIGNAL(itemActivated(CatalogView *, ContentView *, int)), this, SLOT(onOkClicked()));

    hor_layout_.addStretch(0);
    hor_layout_.addWidget(&ok_);


    ver_layout_.addStretch(0);
    ver_layout_.addLayout(&hor_layout_);
}

void PowerManagementDialog::onButtonChanged(CatalogView *catalog, ContentView *item, int user_data)
{
    if (!item || !item->data())
    {
        return;
    }

    OData *selected = item->data();
    if(interval_selected_)
    {
        interval_selected_->insert(TAG_CHECKED, false);
    }
    selected->insert(TAG_CHECKED, true);
    interval_selected_ = selected;

    catalog->update();
    onyx::screen::watcher().enqueue(catalog, onyx::screen::ScreenProxy::GU);
    int i = interval_selected_->value(TITLE_INDEX).toInt();
    standby_interval_ = DISPLAY_ITEMS[i].standby_seconds;
    shutdown_interval_ = DISPLAY_ITEMS[i].shutdown_seconds;
}

bool PowerManagementDialog::event(QEvent* qe)
{
    bool ret = QDialog::event(qe);
    if (qe->type() == QEvent::UpdateRequest
            && onyx::screen::instance().isUpdateEnabled())
    {
         onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::DW);
    }
    return ret;
}

void PowerManagementDialog::onOkClicked()
{
    if (standby_interval_ != sys_standby_interval_)
    {
        status_.setSuspendInterval(standby_interval_);
    }

    if (shutdown_interval_ != sys_shutdown_interval_ )
    {
        status_.setShutdownInterval(shutdown_interval_);
    }
    accept();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
}

}   // namespace ui

