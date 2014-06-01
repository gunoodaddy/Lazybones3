#ifndef INPUTACTION_H
#define INPUTACTION_H

#include <QObject>
#include <QTimer>
#include <QKeySequence>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTextStream>

class InputMethodListModel;

class InputAction : public QObject
{
	Q_OBJECT
	Q_ENUMS(ActionType)

	Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
	Q_PROPERTY(int actionType READ actionType WRITE setActionType NOTIFY actionTypeChanged)
	Q_PROPERTY(int keyCode READ keyCode WRITE setKeyCode NOTIFY keyCodeChanged)
	Q_PROPERTY(int mouseX READ mouseX WRITE setMouseX NOTIFY mouseXChanged)
	Q_PROPERTY(int mouseY READ mouseY WRITE setMouseY NOTIFY mouseYChanged)
	Q_PROPERTY(int delayedMsec READ delayedMsec WRITE setDelayedMsec NOTIFY delayedMsecChanged)
	Q_PROPERTY(bool useLastPos READ useLastPos WRITE setUseLastPos NOTIFY useLastPosChanged)
	Q_PROPERTY(bool running READ running NOTIFY runningChanged)
	Q_PROPERTY(bool keyRecording READ keyRecording WRITE setKeyRecording NOTIFY keyRecordingChanged)

public:
	enum ActionType {
		None,
		MouseLeftClick,
		MouseLeftPress,
		MouseLeftRelease,
		MouseRightClick,
		MouseRightPress,
		MouseRightRelease,
		MouseMove,
		KeyPress,
		KeyRelease,
		KeyInput,
	};

	explicit InputAction(QObject *parent = 0);	
	InputAction(ActionType action, int keyCode, int xPos, int yPos, int delayed);
	virtual ~InputAction(void);

	static void initialize(void);

	static InputAction *fromLineCode(QString line, int &doneIndex);
	QString toLineCode(void);

	void doit(void);
	void doitNow(void);
	Q_INVOKABLE void test(void);
	Q_INVOKABLE void stop(void);
	Q_INVOKABLE QString keyName();

	int actionType(void) { return (int)m_action; }
	void setActionType(int actionType);

	int keyCode(void) { return m_key; }
	void setKeyCode(int keyCode);

	int mouseX(void) { return m_xPos; }
	void setMouseX(int mouseX);

	int mouseY(void) { return m_yPos; }
	void setMouseY(int mouseY);

	int delayedMsec(void) { return m_delayedMsec; }
	void setDelayedMsec(int delayedMsec);

	bool running(void);
	void setRunning(bool running);

	bool keyRecording(void);
	void setKeyRecording(bool keyRecording);

	bool useLastPos(void) { return m_useLastPosition; }
	void setUseLastPos(bool useLastPos) { m_useLastPosition = useLastPos; emit useLastPosChanged(); emit descriptionChanged(); }

	void setModel(InputMethodListModel *model);

	QString description(void);

	bool isKeyAction(void)
	{
		if(m_action == KeyPress || m_action == KeyRelease || m_action == KeyInput)
			return true;
		return false;
	}

	bool isMouseAction(void)
	{
		if(m_action == None)
			return false;
		if(isKeyAction())
			return false;
		return true;
	}

	Q_INVOKABLE void copy(InputAction *action);

	static QSharedPointer<InputAction> fromKeyEvent(const QKeyEvent *event, qint64 delayMsec);
	static QSharedPointer<InputAction> fromMouseEvent(const QMouseEvent *event, qint64 delayMsec);

private:
	qint32 nativeVirtualKey(Qt::Key key);
	void __initTimer(void)
	{
		m_timer.setSingleShot(true);
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTriggered()));
	}

	void __perform(void);
	void __doit(void);
	QString argumentString(void);

signals:
	void actionTypeChanged(void);
	void keyCodeChanged(void);
	void mouseXChanged(void);
	void mouseYChanged(void);
	void delayedMsecChanged(void);
	void runningChanged(void);
	void descriptionChanged(void);
	void keyRecordingChanged(void);
	void useLastPosChanged(void);

public slots:
	void onTriggered(void);
	void onKeyEvent(QKeyEvent *event);

private:
	friend class InputMethodListModel;
	ActionType m_action;
	int m_key;
	int m_xPos;
	int m_yPos;
	int m_delayedMsec;
	bool m_running;
	bool m_doNextAction;
	bool m_keyRecording;
	bool m_useLastPosition;

	QTimer m_timer;
	InputMethodListModel *m_model;
};

#endif // INPUTACTION_H
