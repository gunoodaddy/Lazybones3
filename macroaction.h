#ifndef MACROACTION_H
#define MACROACTION_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QColor>
#include <QTextStream>
#include "inputmethodlistmodel.h"

class MacroActionListModel;
class MacroPresetData;

class MacroAction : public QObject
{
	Q_OBJECT
	Q_ENUMS(ActionType)
	Q_ENUMS(ConditionType)
	Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
	Q_PROPERTY(QString conditionScreen READ conditionScreen WRITE setConditionScreen NOTIFY conditionScreenChanged)
	Q_PROPERTY(int actionType READ actionType WRITE setActionType NOTIFY actionTypeChanged)
	Q_PROPERTY(int conditionType READ conditionType WRITE setConditionType NOTIFY conditionTypeChanged)
	Q_PROPERTY(InputAction* inputAction READ inputAction NOTIFY inputActionChanged)
	Q_PROPERTY(QString comment READ comment WRITE setComment NOTIFY commentChanged)
	Q_PROPERTY(int sleepMsec READ sleepMsec WRITE setSleepMsec NOTIFY sleepMsecChanged)
	Q_PROPERTY(int macroIndex READ macroIndex WRITE setMacroIndex NOTIFY macroIndexChanged)
	Q_PROPERTY(int loopIndex READ loopIndex WRITE setLoopIndex NOTIFY loopIndexChanged)
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
	Q_PROPERTY(int colorOffset READ colorOffset WRITE setColorOffset NOTIFY colorOffsetChanged)
	Q_PROPERTY(QRect colorSearchRect READ colorSearchRect WRITE setColorSearchRect NOTIFY colorSearchRectChanged)
	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
	Q_PROPERTY(bool running READ running NOTIFY runningChanged)
	Q_PROPERTY(bool recording READ recording WRITE setRecording NOTIFY recordingChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(InputMethodListModel * inputActionChunk READ inputActionChunk NOTIFY inputActionChunkChanged)

public:
	enum ConditionType {
		None = 0,
		LoopIfNotStart = 1,
		LoopIfStart = 2,
		LoopEnd = 3,
		IfStart = 4,
		IfNotStart = 5,
		IfEnd = 6,
		Else = 7,
	};

	enum ActionType {
		NoneAction = 0,
		DoInputAction = 1,
		DoInputActionChunk = 11,
		WaitScreen = 9,
		Sleep = 10,
		RandomSleep = 13,
		FindLegendaryItem = 12,
		Goto = 14,
		DisassembleAllItemWithoutLegendary = 15,
		LoopWaitScreenDoInputActionEnd = 16,
		SaveScreenShot = 20,
		CheckElapsedTime = 2,
		CheckDropBoxFileExist = 17,
		CheckLoopIndex = 19,
		CheckLoopIndexModulus = 22,
		CheckScreen = 21,
		CheckPixelColor = 23,
	};

	enum State {
		InitState = 0,
		ConditionState,
		ActionState,
		InputActionState,
		InputActionChunkState,
		ArgumentState,
		CommentState,
		ErrorState,
		DoneState,
	};

	explicit MacroAction(MacroAction *action = 0);

	bool doit();

	Q_INVOKABLE bool blockStartType(void)
	{
		return blockLoopStartType()
				|| blockIfType();
	}

	Q_INVOKABLE bool blockEndType(void)
	{
		return m_conditionType == LoopEnd
				|| m_conditionType == IfEnd;
	}

	Q_INVOKABLE bool isOnlyConditionType(void)
	{
		return blockEndType() || blockRestartType();
	}

	Q_INVOKABLE bool blockRestartType(void)
	{
		return m_conditionType == Else;
	}

	Q_INVOKABLE bool blockLoopStartType(void)
	{
		return m_conditionType == LoopIfStart
				|| m_conditionType == LoopIfNotStart;
	}

	Q_INVOKABLE bool blockLoopEndType(void)
	{
		return m_conditionType == LoopEnd;
	}

	Q_INVOKABLE bool blockIfType(void)
	{
		return checkBlockIfType(m_conditionType);
	}

	Q_INVOKABLE bool negativeConditionType(void)
	{
		return m_conditionType == IfNotStart || m_conditionType == LoopIfNotStart;
	}

	static void initialize(void);

	static bool checkBlockIfType(ConditionType type)
	{
		return type == IfNotStart
				|| type == IfStart;
	}

	static MacroAction *fromTextStream(QTextStream &ts);
	void writeTextStream(QString prefix, QTextStream &ts);
	QString toLineCode(void);

	MacroPresetData *presetData(void);

	void setModel(MacroActionListModel *model) { m_model = model; }

	int actionType(void) { return m_actionType; }
	void setActionType(int type) { m_actionType = (ActionType)type; emit actionTypeChanged(); emit descriptionChanged(); }

	int conditionType(void) { return m_conditionType; }
	void setConditionType(int type) { m_conditionType = (ConditionType)type; emit conditionTypeChanged(); emit descriptionChanged(); }

	const QString &conditionScreen(void) { return m_conditionScreenName; }
	void setConditionScreen(QString name) { m_conditionScreenName = name; emit conditionScreenChanged(); emit descriptionChanged(); }

	const QString &comment(void) { return m_comment; }
	void setComment(QString comment) { m_comment = comment; emit commentChanged(); emit descriptionChanged(); }

	int sleepMsec(void) { return m_sleepMsec; }
	void setSleepMsec(int msec) { m_sleepMsec = msec; emit sleepMsecChanged(); emit descriptionChanged(); }

	int macroIndex(void) { return m_macroIndex; }
	void setMacroIndex(int index) { m_macroIndex = index; emit macroIndexChanged(); emit descriptionChanged(); }

	int loopIndex(void) { return m_loopIndex; }
	void setLoopIndex(int index) { m_loopIndex = index; emit loopIndexChanged(); emit descriptionChanged(); }

	QColor color(void) { return m_color; }
	void setColor(QColor color) { m_color = color; emit colorChanged(); emit descriptionChanged(); }

	int colorOffset(void) { return m_colorOffset; }
	void setColorOffset(int colorOffset) { m_colorOffset = colorOffset; emit colorOffsetChanged(); emit descriptionChanged(); }

	const QRect & colorSearchRect(void) { return m_colorSearchRect; }
	void setColorSearchRect(QRect colorSearchRect) { m_colorSearchRect = colorSearchRect; emit colorSearchRectChanged(); emit descriptionChanged(); }

	bool running(void) { return m_running; }
	void setRunning(bool running);

	bool recording(void) { return m_recording; }
	void setRecording(bool recording);

	bool active(void) { return m_active; }
	void setActive(bool active);

	QString filePath(void) { return m_filePath; }
	void setFilePath(QString name) { m_filePath = name; emit filePathChanged(); emit descriptionChanged(); }

	int realSleepTime(void);

	void init(void);

	Q_INVOKABLE void test(void);

	InputAction* inputAction(void)
	{
		return &m_inputAction;
	}
	InputMethodListModel *inputActionChunk(void) { return &m_inputActionChunk; }

	void addInputActionChunk(InputAction* inputAction);
	Q_INVOKABLE void removeInputActionChunkAt(int index)
	{
		m_inputActionChunk.remove(index);
	}

	QString description(void);

	void copy(MacroAction *action);

private:
	qint64 __calculateDelayMsec(qint64 delayMsec);
	void doFindLegendaryItem(void);
	void doDisassembleAllItemWithoutLegendary(bool testMode = false);
	void doLoopWaitScreenDoInputActionEnd(bool testMode = false);
	void doInputAction(bool testMode = false);
	void doInputActionChunk(bool testMode = false);\
	void doSaveScreenShot(void);

	static void parseLine(MacroAction *newAction, MacroAction::State &state, int & parsedIndex, QString &line);
	QString argumentString(void);
	void setArgumentString(QString arguments);

private slots:
	void onKeyEvent(QKeyEvent *event);
	void onMouseEvent(QMouseEvent *event);

signals:
	void conditionScreenChanged(void);
	void actionTypeChanged(void);
	void conditionTypeChanged(void);
	void inputActionChanged(void);
	void commentChanged(void);
	void sleepMsecChanged(void);
	void macroIndexChanged(void);
	void descriptionChanged(void);
	void runningChanged(void);
	void recordingChanged(void);
	void inputActionChunkChanged(void);
	void activeChanged(void);
	void loopIndexChanged(void);
	void filePathChanged(void);
	void colorChanged(void);
	void colorOffsetChanged(void);
	void colorSearchRectChanged(void);

public slots:
	void onInputActionDescriptionChanged(void)
	{
		emit descriptionChanged();
	}

private:
	MacroActionListModel *m_model;
	bool m_running;
	bool m_recording;

	// Type
    ConditionType m_conditionType;
    ActionType m_actionType;

	// Arguments
	InputAction m_inputAction;
	InputMethodListModel m_inputActionChunk;
	QString m_conditionScreenName;
	int m_sleepMsec;
	int m_macroIndex;
	bool m_active;
	int m_currentSleepMsec;
	int m_loopIndex;
	QString m_filePath;
	QColor m_color;
	int m_colorOffset;
	QRect m_colorSearchRect;

	// Comments
	QString m_comment;

	// Internals
	qint64 m_prevDelayMsec;
	qint64 m_startActionMsec;
};

#endif // MACROACTION_H
