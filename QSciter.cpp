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
#include "sciter-x-key-codes.h"

static UINT SC_CALLBACK handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam);

QSciter::QSciter(QWidget * parent)
	: QWidget(parent)
{	
	/* If this assertion fails it means it could not load the sciter DLL */
	Q_ASSERT(SAPI() != nullptr);
	
	createSciterWindow();

	setMouseTracking(true);
	
	connect(&eventTimer, &QTimer::timeout, this, &QSciter::onTimerEvent);
	eventTimer.setInterval(10);
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

	SciterProcX(sciterHandle, SCITER_X_MSG_CREATE(GFX_LAYER_SKIA_RASTER, FALSE));

	SciterSetCallback(sciterHandle, handle_notification, this);
}

QSciter::~QSciter()
{
#ifdef _WINDOWS
	eventTimer.stop();

	if (sciterHandle != nullptr)
	{
		SciterSetCallback(sciterHandle, NULL, NULL);
		SciterProcX(sciterHandle, SCITER_X_MSG_DESTROY());
	}
		
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
	SciterProcX(sciterHandle, paint);
}

template<class T>
MOUSE_BUTTONS getMouseButtons(T * event)
{
	unsigned mb = 0;

	if (event->button() == Qt::LeftButton)
		mb |= MAIN_MOUSE_BUTTON;
	else if (event->button() == Qt::RightButton)
		mb |= PROP_MOUSE_BUTTON;
	else if (event->button() == Qt::MiddleButton)
		mb |= MIDDLE_MOUSE_BUTTON;

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
		POINT pos = { LONG(event->position().x()), LONG(event->position().y())};
		SciterProcX(sciterHandle, SCITER_X_MSG_MOUSE(evt, getMouseButtons(event), getKeyboardStates(event), pos));
	}

	update();
}

void QSciter::mouseReleaseEvent(QMouseEvent * event)
{
	QWidget::mouseReleaseEvent(event);

	if (sciterHandle != nullptr)
	{
		MOUSE_EVENTS evt = MOUSE_EVENTS::MOUSE_UP;
		POINT pos = { LONG(event->position().x()), LONG(event->position().y()) };
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
		POINT pos = { LONG(event->position().x()), LONG(event->position().y()) };
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
		POINT pos = { LONG(event->position().x()), LONG(event->position().y()) };
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

unsigned int QSciter::translateKey(int vk, quint32 nativeKeycode)
{
#ifdef WIN32
	if (nativeKeycode == VK_RSHIFT)
		return SC_KB_CODES::KB_RIGHT_SHIFT;
	else if (nativeKeycode == VK_SHIFT)
		return SC_KB_CODES::KB_LEFT_SHIFT;
	else if (nativeKeycode == VK_RCONTROL)
		return SC_KB_CODES::KB_RIGHT_CONTROL;
	else if (nativeKeycode == VK_CONTROL)
		return SC_KB_CODES::KB_LEFT_CONTROL;
	else if (nativeKeycode == VK_NAVIGATION_MENU)
		return SC_KB_CODES::KB_CONTEXT_MENU;
#endif

	switch (vk)
	{
	case Qt::Key::Key_Escape: return SC_KB_CODES::KB_ESCAPE;
	case Qt::Key::Key_Enter: return SC_KB_CODES::KB_KP_ENTER;
	case Qt::Key::Key_Return: return SC_KB_CODES::KB_ENTER;
	case Qt::Key::Key_Tab:    return SC_KB_CODES::KB_TAB;
	case Qt::Key::Key_Backspace:   return SC_KB_CODES::KB_BACKSPACE;
	case Qt::Key::Key_Insert:  return SC_KB_CODES::KB_INSERT;
	case Qt::Key::Key_Delete:  return SC_KB_CODES::KB_DELETE;
	case Qt::Key::Key_Right:  return SC_KB_CODES::KB_RIGHT;
	case Qt::Key::Key_Left:  return SC_KB_CODES::KB_LEFT;
	case Qt::Key::Key_Down:  return SC_KB_CODES::KB_DOWN;
	case Qt::Key::Key_Up:  return SC_KB_CODES::KB_UP;
	case Qt::Key::Key_PageUp:  return SC_KB_CODES::KB_PAGE_UP;
	case Qt::Key::Key_PageDown:  return SC_KB_CODES::KB_PAGE_DOWN;
	case Qt::Key::Key_Home:  return SC_KB_CODES::KB_HOME;
	case Qt::Key::Key_End:  return SC_KB_CODES::KB_END;
	case Qt::Key::Key_CapsLock:  return SC_KB_CODES::KB_CAPS_LOCK;
	case Qt::Key::Key_ScrollLock:  return SC_KB_CODES::KB_SCROLL_LOCK;
	case Qt::Key::Key_NumLock:  return SC_KB_CODES::KB_NUM_LOCK;
	case Qt::Key::Key_Print:  return SC_KB_CODES::KB_PRINT_SCREEN;
	case Qt::Key::Key_Pause:  return SC_KB_CODES::KB_PAUSE;

	case Qt::Key::Key_F1:  return SC_KB_CODES::KB_F1;
	case Qt::Key::Key_F2:  return SC_KB_CODES::KB_F2;
	case Qt::Key::Key_F3:  return SC_KB_CODES::KB_F3;
	case Qt::Key::Key_F4:  return SC_KB_CODES::KB_F4;
	case Qt::Key::Key_F5:  return SC_KB_CODES::KB_F5;
	case Qt::Key::Key_F6:  return SC_KB_CODES::KB_F6;
	case Qt::Key::Key_F7:  return SC_KB_CODES::KB_F7;
	case Qt::Key::Key_F8:  return SC_KB_CODES::KB_F8;
	case Qt::Key::Key_F9:  return SC_KB_CODES::KB_F9;
	case Qt::Key::Key_F10:  return SC_KB_CODES::KB_F10;
	case Qt::Key::Key_F11:  return SC_KB_CODES::KB_F11;
	case Qt::Key::Key_F12:  return SC_KB_CODES::KB_F12;
	
	case Qt::Key::Key_F13:  return SC_KB_CODES::KB_F13;
	case Qt::Key::Key_F14:  return SC_KB_CODES::KB_F14;
	case Qt::Key::Key_F15:  return SC_KB_CODES::KB_F15;
	case Qt::Key::Key_F16:  return SC_KB_CODES::KB_F16;
	case Qt::Key::Key_F17:  return SC_KB_CODES::KB_F17;
	case Qt::Key::Key_F18:  return SC_KB_CODES::KB_F18;
	case Qt::Key::Key_F19:  return SC_KB_CODES::KB_F19;
	case Qt::Key::Key_F20:  return SC_KB_CODES::KB_F20;
	case Qt::Key::Key_F21:  return SC_KB_CODES::KB_F21;
	case Qt::Key::Key_F22:  return SC_KB_CODES::KB_F22;
	case Qt::Key::Key_F23:  return SC_KB_CODES::KB_F23;
	case Qt::Key::Key_F24:  return SC_KB_CODES::KB_F24;
	case Qt::Key::Key_F25:  return SC_KB_CODES::KB_F25;

	case Qt::Key::Key_Shift: return SC_KB_CODES::KB_LEFT_SHIFT;
	case Qt::Key::Key_Alt: return SC_KB_CODES::KB_LEFT_ALT;
	case Qt::Key::Key_AltGr: return SC_KB_CODES::KB_RIGHT_ALT;
	case Qt::Key::Key_Menu: return SC_KB_CODES::KB_MENU;
	}
	
	return UINT(vk);
}

void QSciter::keyPressEvent(QKeyEvent * event)
{
	if (sciterHandle != nullptr)
	{
		KEY_EVENTS evt = KEY_EVENTS::KEY_DOWN;
		UINT code = translateKey(event->key(), event->nativeVirtualKey());
		SciterProcX(sciterHandle, SCITER_X_MSG_KEY(evt, code, getKeyboardStates(event)));

		if (!event->text().isEmpty())
		{
			for (QChar c : event->text())
			{
				KEY_EVENTS me = KEY_CHAR;
				SciterProcX(sciterHandle, SCITER_X_MSG_KEY(me, c.unicode(), KEYBOARD_STATES(0)));
			}
		}
	}
	QWidget::keyPressEvent(event);
	update();
}

void QSciter::keyReleaseEvent(QKeyEvent * event)
{
	if (sciterHandle != nullptr)
	{
		KEY_EVENTS evt = KEY_EVENTS::KEY_UP;
		UINT code = translateKey(event->key(), event->nativeVirtualKey());
		SciterProcX(sciterHandle, SCITER_X_MSG_KEY(evt, code, getKeyboardStates(event)));
	}

	QWidget::keyReleaseEvent(event);
	update();
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
		POINT pos = { LONG(event->position().x()), LONG(event->position().y()) };
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

UINT SC_CALLBACK handle_notification(LPSCITER_CALLBACK_NOTIFICATION pnm, LPVOID callbackParam)
{
	QSciter* self = (QSciter*)(callbackParam);
	return self->handleNotification(pnm);
}

unsigned int QSciter::handleNotification(LPSCITER_CALLBACK_NOTIFICATION pnm)
{
	// Crack and call appropriate method
	// here are all notifiactions
	switch (pnm->code) {
	case SC_INVALIDATE_RECT: this->update(); break;
	case SC_SET_CURSOR: this->setSciterCursor((LPSCN_SET_CURSOR)pnm);
	}
	return 0;
}

void QSciter::setSciterCursor(LPSCN_SET_CURSOR pnm)
{
	Qt::CursorShape cursor = Qt::CursorShape::ArrowCursor;
	switch (pnm->cursorId)
	{
	case CURSOR_ARROW:  break;
	case CURSOR_IBEAM:  cursor = Qt::CursorShape::IBeamCursor; break;
	case CURSOR_CROSS:  cursor = Qt::CursorShape::CrossCursor; break;
	case CURSOR_HAND:   cursor = Qt::CursorShape::PointingHandCursor; break;
	case CURSOR_SIZEWE: cursor = Qt::CursorShape::SizeBDiagCursor; break;
	case CURSOR_SIZENS: cursor = Qt::CursorShape::SizeFDiagCursor; break;
	default:
		int a; a = 0;
		break;
	}
	this->setCursor(cursor);
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
