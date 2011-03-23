#ifndef ONYX_SEARCH_DIALOG_H_
#define ONYX_SEARCH_DIALOG_H_

#include "onyx/base/base.h"
#include "ui_global.h"
#include "catalog_view.h"
#include "onyx_dialog.h"
#include "onyx_keyboard.h"

namespace ui
{

/// Define search context. Application may inherit this class
/// to implement its own search context.
class OnyxSearchContext
{
public:
    OnyxSearchContext();
    virtual ~OnyxSearchContext();

public:
    void reset();

    const QString & pattern() const { return pattern_; }
    void setPattern(const QString &pattern);

    bool forward() const { return forward_; }
    void setForward(bool forward = true);

    bool case_sensitive() const { return case_sensitive_; }
    void setCaseSensitive(bool sensitive = true);

    bool match_whole_word() const { return match_whole_word_; }
    void setMatchWholeWord(bool match = true);

    bool isStopped() { return stop_; }
    void stop(bool s = true) { stop_ = s; }

    int userData() const { return user_data_; }
    int & userData() { return user_data_; }

private:
    QString pattern_;
    bool forward_;
    bool case_sensitive_;
    bool match_whole_word_;
    bool stop_;
    int user_data_;
};


/// Search dialog to enable user to search the documents.
// Dialog for getting input with soft keyboard support.
class OnyxSearchDialog: public OnyxDialog
{
    Q_OBJECT

public:
    enum SearchNavigateType {
        SEARCH_NAV_PREVIOUS = 11,
        SEARCH_NAV_NEXT = 12,
    };

public:
    OnyxSearchDialog(QWidget *parent, OnyxSearchContext & ctx);
    ~OnyxSearchDialog();

public:
    int popup(int bottom_margin);
    const QString inputText();
    void adjustPosition();
    void ensureVisible();

public Q_SLOTS:
    void noMoreMatches();

Q_SIGNALS:
    /// The search signal is emitted when user clicked search button,
    /// search next button and search previous button.
    void search(OnyxSearchContext & ctx);

    /// The closeClicked signal is emitted when user close the search
    /// widget. The caller decides to destroy the widget or just hide
    /// the widget.
    void closeClicked();

protected Q_SLOTS:
    void onItemActivated(CatalogView *catalog, ContentView *item,
            int user_data);

private Q_SLOTS:
    void onSearchClicked();
    void onSearchNextClicked();
    void onSearchPrevClicked();
    void onCloseClicked();

private:
    void createLayout();
    void createLineEdit();
    void createSubMenu();
    void createNavigateMenu();
    void connectWithChildren();

    void clearClicked();

    // handling key event
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void updateChildrenWidgets(bool searching);
    void readyToSearch(bool forward);
    void updateTitle(const QString &message = QString());

    void moveEvent(QMoveEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    QVBoxLayout big_layout_;
    QHBoxLayout line_edit_layout_;
    CatalogView line_edit_;
    // contains Search and Clear menu items
    CatalogView sub_menu_;

    OnyxKeyboard keyboard_;

    CatalogView navigate_menu_;

    OnyxSearchContext & ctx_;
    bool full_mode_;
    bool update_parent_;
};

}   // namespace ui

#endif