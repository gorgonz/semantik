// Thomas Nagy 2007-2017 GPLV3


#ifndef CON_h
#define CON_h

#include <QGraphicsItem>
#include "data_item.h"
#include "box_chain.h"

#define MUL 64
#define GRADVAL 110
#define JUST 1
class connectable
{
	public:
		connectable() { m_oBox = NULL; };
		virtual ~connectable() {};
		int m_iId;
		virtual QRectF rect() const { return QRectF(); }
		virtual void update_data() {};
		virtual void update_size() {};
		virtual int choose_position(const QPointF&p, int id=-1) { Q_ASSERT(false); return 0; };
		virtual QPoint get_point(int position) { return QPoint(0, 0); };
		data_box *m_oBox;
		box_chain *m_oChain;


	virtual data_link::Direction pos_heuristic(const QPoint & i_oP, int i_iDirection)
	{
		switch (i_iDirection & data_link::COORD)
		{
			case data_link::NORTH:
				return (i_oP.y() > 0) ? data_link::NORTH : data_link::SOUTH;
				break;
			case data_link::WEST:
				return (i_oP.x() < 0) ? data_link::EAST : data_link::WEST;
				break;
			case data_link::SOUTH:
				return (i_oP.y() < rect().height()) ? data_link::SOUTH : data_link::NORTH;
				break;
			case data_link::EAST:
				return (i_oP.x() > rect().width()) ? data_link::WEST : data_link::EAST;
				break;
			default:
				break;
		}
		return data_link::SOUTH;
	};
};

#endif

