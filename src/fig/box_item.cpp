// Thomas Nagy 2007-2017 GPLV3

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QInputDialog>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QAbstractTextDocumentLayout>
#include <QTextList>
#include <QGraphicsSceneMouseEvent>
#include <QLinearGradient>
#include <QClipboard>
#include <QPainter>
#include <QtDebug>
#include <QAction>
#include <QTextDocument>
#include "box_item.h"
#include "box_view.h"
 #include "box_link.h"
#include "data_item.h"
#include "sem_mediator.h"
#include "mem_box.h"

#define PAD 2

box_item::box_item(box_view* i_oParent, int i_iId) : QGraphicsRectItem(), connectable(), editable(), resizable(), m_oView(i_oParent)
{
	m_iId = i_iId;

	m_oItem = m_oView->m_oMediator->m_oItems[m_oView->m_iId];
	m_oBox = m_oItem->m_oBoxes[m_iId];
	Q_ASSERT(m_oBox);

	i_oParent->scene()->addItem(this);
	m_oChain = new box_chain(i_oParent);
	m_oChain->setParentItem(this);

	setCacheMode(QGraphicsItem::DeviceCoordinateCache);

	m_oResize = new box_resize_point(m_oView, this);
	m_oResize->setRect(-CTRLSIZE - 1, -CTRLSIZE - 1, CTRLSIZE, CTRLSIZE);
	m_oResize->setCursor(Qt::SizeFDiagCursor);
	m_oResize->hide();
	m_oResize->setParentItem(this);

	update_size();
	update_sizers();
	setZValue(100);
	//setCursor(Qt::SizeFDiagCursor);
	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
	x_text_off = 2 * OFF;
	y_text_off = 2 * OFF;
}

box_item::~box_item()
{
	delete m_oChain;
	delete m_oResize;
}

void box_item::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QRectF l_oRect = boundingRect().adjusted(PAD, PAD, -PAD, -PAD);
	painter->setFont(scene()->font());
	doc.setDefaultFont(scene()->font());

	QPen l_oPen = QPen(Qt::SolidLine);

	l_oPen.setColor(Qt::black);
	if (isSelected()) l_oPen.setStyle(Qt::DotLine);
	l_oPen.setCosmetic(false);
	l_oPen.setWidth(1);

	painter->setPen(l_oPen);

	QColor bc(m_oBox->color);
	if (m_oView->m_bDisableGradient)
	{
		painter->setBrush(bc);
	}
	else
	{
		QLinearGradient linearGradient(0, 0, l_oRect.width(), 0);
		linearGradient.setColorAt(0.0, bc);
		linearGradient.setColorAt(1.0, bc.darker(GRADVAL));
		painter->setBrush(linearGradient);
	}


	painter->drawRoundRect(l_oRect, 20, 20);

	QAbstractTextDocumentLayout::PaintContext ctx;
	ctx.palette = QApplication::palette("QTextControl");
	ctx.palette.setColor(QPalette::Text, Qt::black); // white on black kde themes
	QAbstractTextDocumentLayout * lay = doc.documentLayout();
	qreal yoff = lay->documentSize().height();

	painter->translate(OFF, OFF + (rect().height() - 2 * OFF - yoff) / 2.);
	lay->draw(painter, ctx);
}

void box_item::update_data() {
	setPos(QPointF(m_oBox->m_iXX, m_oBox->m_iYY));
	if (m_oBox->m_iWW != m_iWW || m_oBox->m_iHH != m_iHH || doc.toPlainText() != m_oBox->m_sText || doc.textWidth() != m_oBox->m_iWW - x_text_off)
	{
		update_size();
	}
	update();
	update_sizers();
}

void box_item::update_size() {
	m_iWW = m_oBox->m_iWW;
	m_iHH = m_oBox->m_iHH;

	doc.setHtml(QString("<div align='center'>%1</div>").arg(m_oBox->m_sText));
	doc.setTextWidth(m_iWW - x_text_off);

	prepareGeometryChange();
	setRect(0, 0, m_iWW, m_iHH);
	m_oChain->setPos(m_iWW + 3, 0);
	update_links();
	update_sizers();
}

void box_item::properties()
{
	bool ok = false;
	QString text = QInputDialog::getText(m_oView, m_oView->trUtf8("Diagram box properties"),
			m_oView->trUtf8("Text:"), QLineEdit::Normal, m_oBox->m_sText, &ok);
	if (ok && text != m_oBox->m_sText)
	{
		mem_edit_box *ed = new mem_edit_box(m_oView->m_oMediator, m_oView->m_iId, m_iId);
		ed->newText = text;

		QTextDocument doc;
		doc.setHtml(QString("<div align='center'>%1</div>").arg(text));
		doc.setTextWidth(m_oBox->m_iWW - 2 * OFF);
		ed->newHeight = GRID * (((int) (doc.size().height() + 2 * OFF + GRID - 1)) / GRID);
		if (ed->newHeight < m_oBox->m_iHH)
			ed->newHeight = m_oBox->m_iHH;

		ed->apply();
	}
}

QVariant box_item::itemChange(GraphicsItemChange i_oChange, const QVariant &i_oValue)
{
	if (scene())
	{
		if (i_oChange == ItemPositionChange)
		{
			QPointF np = i_oValue.toPointF();
			np.setX(((int) np.x() / GRID) * GRID);
			np.setY(((int) np.y() / GRID) * GRID);
			return np;
		}
		else if (i_oChange == ItemPositionHasChanged)
		{
			update_links();
			update_sizers();
		}
		else if (i_oChange == ItemSelectedHasChanged)
		{
			update_selection();
		}
	}

	return QGraphicsItem::itemChange(i_oChange, i_oValue);
}

void box_item::update_selection()
{
	m_oChain->setVisible(isSelected());
	m_oResize->setVisible(isSelected());
}

void box_item::update_links()
{
	// FIXME
	foreach (box_link* l_oLink, m_oView->m_oLinks)
	{
		l_oLink->update_pos();
	}
}

static int RATIO[] = {333, 500, 667, 0};

int box_item::choose_position(const QPointF& i_oP, int id)
{
	QRectF r = rect();
	QPointF l_o = pos() - i_oP + QPointF(r.width()/2, r.height()/2);
	double c_x = l_o.x() * r.height();
	double c_y = l_o.y() * r.width();

	int ret = 0;
	int best = 1<<30;
	int cand = 0;
	if (qAbs(c_x) > qAbs(c_y))
	{
		ret = (c_x > 0) ? data_link::WEST : data_link::EAST;
		for (int i=0; i < 10; ++i) {
			int k = RATIO[i];
			if (k == 0) break;
			int val = qAbs((k * r.height() / 1000.) - (i_oP.y() - pos().y()));

			if (val < best)
			{
				best = val;
				cand = k;
			}
		}
		ret += cand * MUL;
	}
	else
	{
		ret = (c_y > 0) ? data_link::NORTH : data_link::SOUTH;
		for (int i=0; i < 10; ++i) {
			int k = RATIO[i];
			if (k == 0) break;
			int val = qAbs((k * r.width() / 1000.) - (i_oP.x() - pos().x()));

			if (val < best)
			{
				best = val;
				cand = k;
			}
		}
		ret += cand * MUL;
	}
	return ret;
}

QPoint box_item::get_point(int i_oP)
{
	QRectF r = rect();
	int ratio = i_oP / MUL;

	if (ratio >= 1000 || ratio <= 0) ratio = 500;
	switch (i_oP & data_link::COORD) {
		case data_link::NORTH:
			return QPoint(r.x() + r.width() * ratio / 1000., r.y());
		case data_link::WEST:
			return QPoint(r.x(), r.y() + r.height() * ratio / 1000.);
		case data_link::SOUTH:
			return QPoint(r.x() + r.width() * ratio / 1000., r.y() + r.height());
		case data_link::EAST:
			return QPoint(r.x() + r.width(), r.y() + r.height() * ratio / 1000.);
	}
	Q_ASSERT(false);
	return QPoint(0, 0);
}

QSize box_item::best_size(const QPointF &dims)
{
	int x = dims.x();
	x = GRID * (x / GRID);
	if (x < GRID) x = GRID;

	while (true) {
		int l_oWantedW = x - x_text_off;
		doc.setTextWidth(l_oWantedW);
		if (doc.size().width() <= l_oWantedW)
			break;
		x += GRID;
	}

	int y = dims.y();
	y = GRID * (y / GRID);
	if (y < GRID) y = GRID;
	while (y - y_text_off < doc.size().height())
	{
		y += GRID;
	}
	return QSize(x, y);
}

QPointF box_item::validate_point(box_resize_point *p, const QPointF & orig)
{
	QSize s = best_size(orig);
	m_iLastStretchX = s.width();
	m_iLastStretchY = s.height();
	m_oChain->setPos(m_iLastStretchX + 3, 0);

	prepareGeometryChange();
	setRect(0, 0, m_iLastStretchX, m_iLastStretchY);
	update();
	update_links();
	m_oView->message(m_oView->trUtf8("%1 x %2").arg(QString::number(m_iLastStretchX), QString::number(m_iLastStretchY)), 1000);
	return QPointF(m_iLastStretchX, m_iLastStretchY);
}

void box_item::commit_size(box_resize_point *p)
{
	QRect r_orig(m_oBox->m_iXX, m_oBox->m_iYY, m_oBox->m_iWW, m_oBox->m_iHH);
	QRect r_dest(m_oBox->m_iXX, m_oBox->m_iYY, m_iLastStretchX, m_iLastStretchY);

	if (r_orig != r_dest)
	{
		mem_size_box *mem = new mem_size_box(m_oView->m_oMediator, m_oView->m_iId);
		mem->prev_values[m_oBox] = r_orig;
		mem->next_values[m_oBox] = r_dest;
		mem->apply();
	}
}

void box_item::freeze(bool b)
{
	if (b)
	{
		setFlags(ItemIsSelectable);
		m_iLastStretchX = 0;
		m_iLastStretchY = 0;
	}
	else
	{
		setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
	}
}

void box_item::update_sizers()
{
	m_oResize->setPos(m_oBox->m_iWW, m_oBox->m_iHH);
}

