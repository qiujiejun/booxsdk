#ifndef ONYX_CATALOG_VIEW_H_
#define ONYX_CATALOG_VIEW_H_

#include <QGridLayout>
#include "content_view.h"
#include "paginator.h"
#include "data.h"
#include "factory.h"

class CatalogView : public QWidget
{
    Q_OBJECT

public:
    CatalogView(Factory * factory = 0, QWidget *parent = 0);
    ~CatalogView();

public slots:
    void setChecked(bool checked = true);
    bool isChecked() { return checked_; }

    void setSelfRecycle(bool r = true);
    bool isSelfRecycle();

    void goPrev();
    void goNext();
    void gotoPage(const int);

    void setMargin(int m = 4) { margin_ = m; }
    int margin() { return margin_; }

    void showBorder(bool show = true) { show_border_ = show; }
    bool hasBorder() { return show_border_; }

    void setData(const ODatas &list);
    ODatas & data();

    void setFocusTo(const int row, const int col);
    ContentView *focusItem();

    void setFixedGrid(int rows, int cols);
    bool isFixedGrid();
    QSize preferItemSize();
    void setPreferItemSize(const QSize &size);
    int rows();
    int cols();

    void setNeighbor(CatalogView *neighbor, const QString& type);

public:
    static const QString LEFT;
    static const QString RIGHT;
    static const QString UP;
    static const QString DOWN;
    static const QString RECYCLE_LEFT;
    static const QString RECYCLE_RIGHT;
    static const QString RECYCLE_UP;
    static const QString RECYCLE_DOWN;

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent *);
    void changeEvent(QEvent *event);
    void resizeEvent(QResizeEvent * event);
    void paintEvent(QPaintEvent * event);

Q_SIGNALS:
    void cursorMoved(int old, int new_pos);
    void positionChanged(const int, const int);
    void itemActivated(ContentView *item);
    void keyRelease(CatalogView *view, QKeyEvent *key);

    void outOfLeft(CatalogView*, int, int, bool &);
    void outOfRight(CatalogView*, int, int, bool &);
    void outOfUp(CatalogView*, int, int, bool &);
    void outOfDown(CatalogView*, int, int, bool &);

private slots:
    void onItemActivated(ContentView *item);
    void onItemKeyRelease(ContentView *item, QKeyEvent *key);
    void onMouseMoved(QPoint last, QPoint current);
    QWidget * moveFocus(int index);
    ContentView *createSubItem();

private:
    typedef QVector<CatalogView *> CatalogViews;

private:
    void createLayout();
    void calculateLayout(int &rows, int &cols);
    void arrangeSubWidgets();

    void associateData();
    void associateEmptyData();
    void arrangeAll();

    int moveLeft(int current);
    int moveRight(int current);
    int moveUp(int current);
    int moveDown(int current);

    CatalogViews & neighbors(const QString &type);
    bool addNeighbor(const QString &type, CatalogView *neighbor);
    bool searchNeighbors(const QString &type);
    ContentView* findShortestItem(CatalogView *view, QWidget *target, int & dist, const QPoint &offset);
    QString invert(const QString &type);

    int row(int index);
    int col(int index);

    Paginator & paginator();
    void resetPaginator(bool sync_layout = false);
    void broadcastPositionSignal();

private:
    QGridLayout layout_;
    QVector<ContentView *> sub_items_;
    Factory * factory_;
    ODatas datas_;
    int margin_;
    bool checked_;
    bool self_recycle_;
    bool show_border_;
    bool fixed_grid_;
    QSize size_;
    Paginator paginator_;
    QMap<QString, CatalogViews> neighbors_;
};

#endif