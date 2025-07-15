#pragma once

#include <QObject>
#include <QStack>
#include <QStringList>
#include <QLoggingCategory>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(cadCommands)

/**
 * @brief Abstract base class for all CAD commands
 * 
 * Implements the Command pattern for undo/redo functionality.
 * All drawing and modification operations should inherit from this class.
 */
class CADCommand
{
public:
    virtual ~CADCommand() = default;
    
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void redo() { execute(); }
    
    virtual QString name() const = 0;
    virtual QString description() const { return name(); }
    
    virtual bool canUndo() const { return true; }
    virtual bool canRedo() const { return true; }
    
    // Command grouping for macro operations
    virtual bool isGroupStart() const { return false; }
    virtual bool isGroupEnd() const { return false; }
    virtual QString groupName() const { return QString(); }
};

/**
 * @brief Command group for batching multiple operations
 */
class CADCommandGroup : public CADCommand
{
public:
    explicit CADCommandGroup(const QString& name);
    ~CADCommandGroup() override;
    
    void addCommand(std::unique_ptr<CADCommand> command);
    void clear();
    
    void execute() override;
    void undo() override;
    void redo() override;
    
    QString name() const override { return m_name; }
    QString description() const override;
    
    bool isEmpty() const { return m_commands.empty(); }
    size_t commandCount() const { return m_commands.size(); }

private:
    QString m_name;
    std::vector<std::unique_ptr<CADCommand>> m_commands;
    bool m_executed;
};

/**
 * @brief Manages command execution, undo/redo functionality
 * 
 * Provides comprehensive command management including:
 * - Command execution with parameter validation
 * - Unlimited undo/redo with command history
 * - Command grouping for complex operations
 * - Command aliases and shortcuts
 * - Script execution and recording
 * - Command completion and suggestions
 */
class CommandManager : public QObject
{
    Q_OBJECT

public:
    explicit CommandManager(QObject *parent = nullptr);
    ~CommandManager();

    // Command execution
    bool executeCommand(const QString& commandLine);
    bool executeCommand(std::unique_ptr<CADCommand> command);
    
    // Undo/Redo operations
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void clearHistory();
    
    // Command history
    QStringList getUndoHistory() const;
    QStringList getRedoHistory() const;
    QString getLastCommand() const;
    
    // Command grouping
    void beginGroup(const QString& groupName);
    void endGroup();
    bool isGrouping() const { return m_currentGroup != nullptr; }
    
    // Command registration and aliases
    void registerCommand(const QString& name, std::function<std::unique_ptr<CADCommand>(const QStringList&)> factory);
    void registerAlias(const QString& alias, const QString& command);
    void unregisterCommand(const QString& name);
    void unregisterAlias(const QString& alias);
    
    // Command completion and help
    QStringList getAvailableCommands() const;
    QStringList getCommandCompletions(const QString& partial) const;
    QString getCommandHelp(const QString& command) const;
    
    // Script execution
    bool executeScript(const QString& scriptPath);
    bool executeScriptText(const QString& scriptText);
    
    // Command recording
    void startRecording(const QString& macroName);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    void playMacro(const QString& macroName);
    QStringList getAvailableMacros() const;
    
    // Settings
    void setUndoLimit(int limit);
    int undoLimit() const { return m_undoLimit; }
    
    void setCommandEcho(bool echo) { m_commandEcho = echo; }
    bool commandEcho() const { return m_commandEcho; }

signals:
    void commandExecuted(const QString& command);
    void commandFailed(const QString& command, const QString& error);
    void undoAvailabilityChanged(bool available);
    void redoAvailabilityChanged(bool available);
    void historyChanged();
    void groupingChanged(bool grouping);

public slots:
    void repeatLastCommand();
    void cancelCurrentCommand();

private slots:
    void onCommandCompleted();
    void onCommandFailed(const QString& error);

private:
    struct CommandInfo {
        QString name;
        QString help;
        std::function<std::unique_ptr<CADCommand>(const QStringList&)> factory;
    };
    
    void addToHistory(std::unique_ptr<CADCommand> command);
    void trimHistory();
    void updateUndoRedoState();
    
    QStringList parseCommandLine(const QString& commandLine);
    QString resolveAlias(const QString& command);
    std::unique_ptr<CADCommand> createCommand(const QString& name, const QStringList& args);
    
    void initializeBuiltinCommands();
    void registerBuiltinCommand(const QString& name, const QString& help,
                               std::function<std::unique_ptr<CADCommand>(const QStringList&)> factory);

    // Command history stacks
    QStack<std::unique_ptr<CADCommand>> m_undoStack;
    QStack<std::unique_ptr<CADCommand>> m_redoStack;
    
    // Command registration
    QHash<QString, CommandInfo> m_commands;
    QHash<QString, QString> m_aliases;
    
    // Command grouping
    std::unique_ptr<CADCommandGroup> m_currentGroup;
    
    // Command recording
    bool m_recording;
    QString m_currentMacroName;
    QStringList m_recordedCommands;
    QHash<QString, QStringList> m_macros;
    
    // Settings
    int m_undoLimit;
    bool m_commandEcho;
    
    // State
    QString m_lastCommand;
    bool m_commandInProgress;
};
