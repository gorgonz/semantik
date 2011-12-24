// Thomas Nagy 2007-2011 GPLV3


#ifndef BOX_VIEW_H
#define BOX_VIEW_H

#include <QGraphicsView>
#include <QList>
#include <QPoint>
#include "con.h"

class QActionGroup;
class box_item;
class box_link;
class sem_model;
class box_view : public QGraphicsView
{
	Q_OBJECT
	public:
		box_view(QWidget*, sem_model *);
		~box_view();


		QList<box_item*> selection();
		void clear_diagram();

		QList<QGraphicsItem*> m_oSelected;
		QList<box_link*> m_oLinks;

		QMap<int, box_item*> m_oItems;


		void deselect_all();
		void add_select(QGraphicsItem*);
		void rm_select(QGraphicsItem*);

		QPointF m_oLastPoint;
		QPointF m_oLastMovePoint;
		QPoint m_oScrollPoint;
		bool m_bPressed;

		QPointF m_oOffsetPoint;

		void check_canvas_size();

		QMenu* m_oMenu;
		QMenu* m_oWidthMenu;
		QMenu* m_oStyleMenu;

		QAction *m_oAddItemAction;
		QAction *m_oEditAction;
		QAction *m_oDeleteAction;
		QAction *m_oColorAction;
		QAction *m_oMoveUpAction;
		QAction *m_oMoveDownAction;

		void focusInEvent(QFocusEvent *);
		void focusOutEvent(QFocusEvent *);

		void enable_menu_actions(); // like check_actions, but only for the popup menu

		int next_id();

		int m_iId; // the item this diagram belongs to

		// private
		int m_iIdCounter;

		void sync_view();
		void from_string(const QString &);

		sem_model *m_oControl;

		box_link *m_oCurrent;

		int m_bScroll;

		QActionGroup *m_oWidthGroup;
		QActionGroup *m_oStyleGroup;


		void mousePressEvent2(QMouseEvent *);
		void mouseMoveEvent2(QMouseEvent*);
		void mouseReleaseEvent2(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void wheelEvent(QWheelEvent*);

	public slots:
		void enable_actions(); // used on focus in
		void slot_delete();
		void slot_add_item();
		void slot_color();
		void slot_toggle_edit();

		void slot_move_up();
		void slot_move_down();

		void slot_penstyle();
		void slot_penwidth();

		void notify_add_item(int);
		void notify_add_box(int, int);
		void notify_del_box(int, int);

		void notify_select(const QList<int>& unsel, const QList<int>& sel);
		void notify_export_item(int);
};

#endif

