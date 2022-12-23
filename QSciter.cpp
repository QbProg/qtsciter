#include "QSciter.h"
#include <QTimer>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

#define WINDOWLESS
#include "sciter-x-api.h"
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-dom.h"

QSciter::QSciter(QWidget * parent)
	: QWidget(parent)
{	
	/* If this assertion fails it means it could not load the sciter DLL */
	Q_ASSERT(SAPI() != nullptr);
	
	createSciterWindow();

	setMouseTracking(true);

	connect(&eventTimer, &QTimer::timeout, this, &QSciter::onTimerEvent);
	eventTimer.setInterval(5);
	eventTimer.start();
}

void QSciter::createSciterWindow()
{
	if (sciterHandle != nullptr)
		return;

	sciterHandle = this;

	// Create Sciter child window with size exactly as our control.
	SciterSetOption(NULL, SCITER_SET_GFX_LAYER, GFX_LAYER_SKIA);
	SciterSetOption(NULL, SCITER_SET_UX_THEMING, TRUE);

	// Enable debug
#if 1 
		SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES,
			ALLOW_FILE_IO |
			ALLOW_SOCKET_IO |
			ALLOW_EVAL |
			ALLOW_SYSINFO);
		SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
#endif

	auto ok = SciterProcX(sciterHandle, SCITER_X_MSG_CREATE(GFX_LAYER_SKIA_RASTER, FALSE));
}

QSciter::~QSciter()
{
#ifdef _WINDOWS
	eventTimer.stop();

	if (sciterHandle != nullptr)
		SciterProcX(sciterHandle, SCITER_X_MSG_DESTROY());
		
	sciterHandle = nullptr;
#endif
}

void QSciter::resizeEvent(QResizeEvent * resevent)
{
	if (sciterHandle == nullptr)
		return;

	int width = this->width();
	int height = this->height();
	SciterProcX(sciterHandle, SCITER_X_MSG_SIZE(width, height));
}

void QSciter::paintEvent(QPaintEvent * resevent)
{
	if (sciterHandle == nullptr)
	{
		return QWidget::paintEvent(resevent);
	}

	QPainter painter(this);

	auto on_bitmap = [](LPCBYTE rgba, INT x, INT y, UINT width, UINT height, LPVOID param)
	{
		QPainter * painter = (QPainter *)param;
		QImage img(rgba, width, height, QImage::Format::Format_ARGB32);
		//img.save("C:/screen.png");

		painter->drawImage(QPoint{ x,y }, img);
	};

	SCITER_X_MSG_PAINT paint;
	paint.targetType = SPT_RECEIVER;
	paint.target.receiver.param = &painter;
	paint.target.receiver.callback = on_bitmap;
	auto ok = SciterProcX(sciterHandle, paint);
}

template<class T>
MOUSE_BUTTONS getMouseButtons(T * event)
{
	unsigned mb = 0;
	if (event->buttons().testFlag(Qt::RightButton))
		mb |= PROP_MOUSE_BUTTON;
	if (event->buttons().testFlag(Qt::LeftButton))
		mb |= MAIN_MOUSE_BUTTON;
	if (event->buttons().testFlag(Qt::MiddleButton))
		mb |= MIDDLE_MOUSE_BUTTON;
	return MOUSE_BUTTONS(mb);
};

template<class T>
KEYBOARD_STATES getKeyboardStates(T * event)
{
	unsigned ks = 0;
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
		ks |= KEYBOARD_STATES::SHIFT_KEY_PRESSED;
	if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
		ks |= KEYBOARD_STATES::CONTROL_KEY_PRESSED;
	if (event->modifiers().testFlag(Qt::KeyboardModifier::AltModifier))
		ks |= KEYBOARD_STATES::ALT_KEY_PRESSED;
	return KEYBOARD_STATES(ks);
}

void QSciter::mousePressEvent(QMouseEvent * event)
{
	QWidget::mousePressEvent(event);
	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_DOWN;
		POINT pos = { event->x(), event->y() };
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));

		// @this makes click work but not double click
#if 0
		// @todo not clear
		evt = MOUSE_EVENTS::MOUSE_UP;
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));
#endif
	}

	update();
}

void QSciter::mouseReleaseEvent(QMouseEvent * event)
{
	QWidget::mouseReleaseEvent(event);

	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_UP;
		POINT pos = { event->x(), event->y() };
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));

		update();
	}	
}

void QSciter::mouseDoubleClickEvent(QMouseEvent * event)
{
	QWidget::mouseDoubleClickEvent(event);

	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_DCLICK;
		POINT pos = { event->x(), event->y() };
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));

		update();
	}
}

void QSciter::mouseMoveEvent(QMouseEvent * event)
{
	QWidget::mouseMoveEvent(event);

	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_MOVE;
		POINT pos = { event->x(), event->y() };
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));
	}
}

void QSciter::wheelEvent(QWheelEvent * event)
{
	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_WHEEL;
		POINT pos = { LONG(event->position().x()), LONG(event->position().y()) };
		double dx = event->angleDelta().x() * 1;
		double dy = event->angleDelta().y() * 1;
		UINT deltas = short((short(dx) << 16));
		deltas &= 0xFFFF0000;
		deltas |= ((short)dy) & 0xFFFF;
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, MOUSE_BUTTONS(deltas), KEYBOARD_STATES(0), pos));
	}
	QWidget::wheelEvent(event);
	update();
}

void QSciter::keyPressEvent(QKeyEvent * event)
{
	if (sciterHandle != nullptr)
	{
#if 0 // @todo! need to translate keys
		KEY_EVENTS evt = KEY_EVENTS::KEY_DOWN;
		UINT code = event->key();
		SciterProcX(sciterHandle, SCITER_X_MSG_KEY(evt, code, getKeyboardStates(event)));
#endif
	}
	QWidget::keyPressEvent(event);
}

void QSciter::keyReleaseEvent(QKeyEvent * event)
{
	if (sciterHandle != nullptr)
	{
		KEY_EVENTS evt = KEY_EVENTS::KEY_UP;
		UINT code = event->key();
		SciterProcX(sciterHandle, SCITER_X_MSG_KEY(evt, code, getKeyboardStates(event)));
	}
	QWidget::keyPressEvent(event);
}

void QSciter::focusInEvent(QFocusEvent * event)
{
	if (sciterHandle != nullptr)
	{
		SciterProcX(sciterHandle, SCITER_X_MSG_FOCUS(true));
	}
	QWidget::focusInEvent(event);
}

void QSciter::focusOutEvent(QFocusEvent * event)
{
	if (sciterHandle != nullptr)
	{
		SciterProcX(sciterHandle, SCITER_X_MSG_FOCUS(false));
	}
}

void QSciter::enterEvent(QEnterEvent * event)
{
	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_ENTER;
		POINT pos = { event->x(), event->y() };
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));
	}
	QWidget::enterEvent(event);
}

void QSciter::leaveEvent(QEvent * event)
{
	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_LEAVE;
		POINT pos = { 0, 0 };
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, MOUSE_BUTTONS(0), KEYBOARD_STATES(0), pos));
	}
	QWidget::leaveEvent(event);
}


QSize QSciter::sizeHint() const
{
	return QSize(800, 600);
}

void QSciter::onTimerEvent()
{
	if (sciterHandle != nullptr)
		SciterProcX(sciterHandle, SCITER_X_MSG_HEARTBIT(GetTickCount()));
}

void QSciter::load(const QUrl & url)
{
	SciterLoadFile(sciterHandle, url.toString().toStdWString().c_str());
	
	this->update();
}
