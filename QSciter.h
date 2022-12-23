#pragma once
#include <QWidget>
#include <QUrl>
#include <QTimer>

/**
*	Sciter widget
*/
class QSciter : public QWidget
{
	Q_OBJECT;

	/** Handle of the sciter instance */
	void * sciterHandle = nullptr;

	/** Heartbeat timer */
	QTimer eventTimer;

public:

	/** Constructor and destructor */
	QSciter(QWidget * parent = nullptr);
	~QSciter();

	/** Loads the URL into the sciter window */
	void load(const QUrl & url);

	QSize sizeHint() const override;

protected:

	/** Create scite rwindow */
	void createSciterWindow();

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

private slots:
	void onTimerEvent();
};