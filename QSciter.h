#pragma once
#include <QWidget>
#include <QUrl>
#include <QTimer>

/* Forward declarations */
struct SCITER_CALLBACK_NOTIFICATION;
typedef SCITER_CALLBACK_NOTIFICATION* LPSCITER_CALLBACK_NOTIFICATION;
struct SCN_SET_CURSOR;
typedef SCN_SET_CURSOR* LPSCN_SET_CURSOR;

/**
*	Sciter widget
*/
class QSciter : public QWidget
{
	Q_OBJECT;

	/** Heartbeat timer */
	QTimer eventTimer;

protected:
	
	/** Handle of the sciter instance */
	void* sciterHandle = nullptr;

public:

	/** Constructor and destructor */
	QSciter(QWidget * parent = nullptr);
	~QSciter();

	/** Loads the URL into the sciter window */
	void load(const QUrl & url);

	/** Public notification handler: call if callback gets replaced */
	unsigned int handleNotification(LPSCITER_CALLBACK_NOTIFICATION pnm);

	/** returns the sciter handle */
	inline void * handle()
	{
		return sciterHandle;
	}
private:

	/** Creates sciter window */
	void createSciterWindow();

	/** Translate qt key to sciter code */
	unsigned int translateKey(int vk, quint32 nativeKeycode);

	/** Sets the cursor */
	void setSciterCursor(LPSCN_SET_CURSOR cursor);

private slots:
	/** Heartbeat */
	void onTimerEvent();

protected:
	void resizeEvent(QResizeEvent * resevent) override;
	void paintEvent(QPaintEvent * resevent) override;
	void mousePressEvent(QMouseEvent * event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;
	void mouseDoubleClickEvent(QMouseEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;
	void wheelEvent(QWheelEvent * event) override;
	void keyPressEvent(QKeyEvent * event) override;
	void keyReleaseEvent(QKeyEvent * event) override;
	void focusInEvent(QFocusEvent * event) override;
	void focusOutEvent(QFocusEvent * event) override;
	void enterEvent(QEnterEvent * event) override;
	void leaveEvent(QEvent * event) override;

};