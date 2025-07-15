#include "CommandManager.h"
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>

Q_LOGGING_CATEGORY(cadCommands, "cad.commands")

// CADCommandGroup implementation
CADCommandGroup::CADCommandGroup(const QString& name)
    : m_name(name)
    , m_executed(false)
{
}

CADCommandGroup::~CADCommandGroup() = default;

void CADCommandGroup::addCommand(std::unique_ptr<CADCommand> command)
{
    if (command) {
        m_commands.push_back(std::move(command));
    }
}

void CADCommandGroup::clear()
{
    m_commands.clear();
    m_executed = false;
}

void CADCommandGroup::execute()
{
    qCDebug(cadCommands) << "Executing command group:" << m_name << "with" << m_commands.size() << "commands";
    
    for (auto& command : m_commands) {
        command->execute();
    }
    m_executed = true;
}

void CADCommandGroup::undo()
{
    qCDebug(cadCommands) << "Undoing command group:" << m_name;
    
    // Undo in reverse order
    for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
        (*it)->undo();
    }
    m_executed = false;
}

void CADCommandGroup::redo()
{
    qCDebug(cadCommands) << "Redoing command group:" << m_name;
    execute();
}

QString CADCommandGroup::description() const
{
    return QString("%1 (%2 commands)").arg(m_name).arg(m_commands.size());
}

// CommandManager implementation
CommandManager::CommandManager(QObject *parent)
    : QObject(parent)
    , m_recording(false)
    , m_undoLimit(100)
    , m_commandEcho(true)
    , m_commandInProgress(false)
{
    qCDebug(cadCommands) << "Command manager created";
    
    initializeBuiltinCommands();
    
    // Load saved macros
    QString macroPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/macros";
    QDir().mkpath(macroPath);
}

CommandManager::~CommandManager()
{
    qCDebug(cadCommands) << "Command manager destroyed";
    clearHistory();
}

bool CommandManager::executeCommand(const QString& commandLine)
{
    if (commandLine.trimmed().isEmpty()) {
        return false;
    }
    
    qCDebug(cadCommands) << "Executing command line:" << commandLine;
    
    if (m_commandEcho) {
        emit commandExecuted(commandLine);
    }
    
    // Record command if recording
    if (m_recording) {
        m_recordedCommands.append(commandLine);
    }
    
    // Parse command line
    QStringList parts = parseCommandLine(commandLine);
    if (parts.isEmpty()) {
        return false;
    }
    
    QString commandName = parts.first().toLower();
    QStringList args = parts.mid(1);
    
    // Resolve aliases
    commandName = resolveAlias(commandName);
    
    // Handle special commands
    if (commandName == "u" || commandName == "undo") {
        undo();
        return true;
    } else if (commandName == "redo") {
        redo();
        return true;
    } else if (commandName == "repeat" || commandName.isEmpty()) {
        repeatLastCommand();
        return true;
    }
    
    // Create and execute command
    try {
        auto command = createCommand(commandName, args);
        if (command) {
            return executeCommand(std::move(command));
        } else {
            QString error = QString("Unknown command: %1").arg(commandName);
            qCWarning(cadCommands) << error;
            emit commandFailed(commandLine, error);
            return false;
        }
    } catch (const std::exception& e) {
        QString error = QString("Command execution failed: %1").arg(e.what());
        qCWarning(cadCommands) << error;
        emit commandFailed(commandLine, error);
        return false;
    }
}

bool CommandManager::executeCommand(std::unique_ptr<CADCommand> command)
{
    if (!command) {
        return false;
    }
    
    qCDebug(cadCommands) << "Executing command:" << command->name();
    
    try {
        m_commandInProgress = true;
        command->execute();
        m_commandInProgress = false;
        
        // Add to history if not grouping
        if (m_currentGroup) {
            m_currentGroup->addCommand(std::move(command));
        } else {
            addToHistory(std::move(command));
        }
        
        m_lastCommand = command->name();
        return true;
    } catch (const std::exception& e) {
        m_commandInProgress = false;
        QString error = QString("Command execution failed: %1").arg(e.what());
        qCWarning(cadCommands) << error;
        emit commandFailed(command->name(), error);
        return false;
    }
}

bool CommandManager::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool CommandManager::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void CommandManager::undo()
{
    if (!canUndo()) {
        qCWarning(cadCommands) << "Cannot undo: no commands in history";
        return;
    }
    
    auto command = std::move(m_undoStack.top());
    m_undoStack.pop();
    
    qCDebug(cadCommands) << "Undoing command:" << command->name();
    
    try {
        command->undo();
        m_redoStack.push(std::move(command));
        updateUndoRedoState();
        emit historyChanged();
    } catch (const std::exception& e) {
        qCWarning(cadCommands) << "Undo failed:" << e.what();
        // Put command back on undo stack
        m_undoStack.push(std::move(command));
    }
}

void CommandManager::redo()
{
    if (!canRedo()) {
        qCWarning(cadCommands) << "Cannot redo: no commands in redo stack";
        return;
    }
    
    auto command = std::move(m_redoStack.top());
    m_redoStack.pop();
    
    qCDebug(cadCommands) << "Redoing command:" << command->name();
    
    try {
        command->redo();
        m_undoStack.push(std::move(command));
        updateUndoRedoState();
        emit historyChanged();
    } catch (const std::exception& e) {
        qCWarning(cadCommands) << "Redo failed:" << e.what();
        // Put command back on redo stack
        m_redoStack.push(std::move(command));
    }
}

void CommandManager::clearHistory()
{
    qCDebug(cadCommands) << "Clearing command history";
    
    m_undoStack.clear();
    m_redoStack.clear();
    updateUndoRedoState();
    emit historyChanged();
}

QStringList CommandManager::getUndoHistory() const
{
    QStringList history;
    
    // Convert stack to list (most recent first)
    QStack<std::unique_ptr<CADCommand>> tempStack = m_undoStack;
    while (!tempStack.isEmpty()) {
        auto& command = tempStack.top();
        history.prepend(command->description());
        tempStack.pop();
    }
    
    return history;
}

QStringList CommandManager::getRedoHistory() const
{
    QStringList history;
    
    // Convert stack to list (most recent first)
    QStack<std::unique_ptr<CADCommand>> tempStack = m_redoStack;
    while (!tempStack.isEmpty()) {
        auto& command = tempStack.top();
        history.append(command->description());
        tempStack.pop();
    }
    
    return history;
}

QString CommandManager::getLastCommand() const
{
    return m_lastCommand;
}

void CommandManager::beginGroup(const QString& groupName)
{
    qCDebug(cadCommands) << "Beginning command group:" << groupName;
    
    if (m_currentGroup) {
        qCWarning(cadCommands) << "Already in a command group, ending previous group";
        endGroup();
    }
    
    m_currentGroup = std::make_unique<CADCommandGroup>(groupName);
    emit groupingChanged(true);
}

void CommandManager::endGroup()
{
    if (!m_currentGroup) {
        qCWarning(cadCommands) << "Not in a command group";
        return;
    }
    
    qCDebug(cadCommands) << "Ending command group:" << m_currentGroup->name();
    
    if (!m_currentGroup->isEmpty()) {
        addToHistory(std::move(m_currentGroup));
    }
    
    m_currentGroup.reset();
    emit groupingChanged(false);
}

void CommandManager::registerCommand(const QString& name, std::function<std::unique_ptr<CADCommand>(const QStringList&)> factory)
{
    qCDebug(cadCommands) << "Registering command:" << name;
    
    CommandInfo info;
    info.name = name;
    info.factory = factory;
    
    m_commands[name.toLower()] = info;
}

void CommandManager::registerAlias(const QString& alias, const QString& command)
{
    qCDebug(cadCommands) << "Registering alias:" << alias << "->" << command;
    m_aliases[alias.toLower()] = command.toLower();
}

void CommandManager::unregisterCommand(const QString& name)
{
    qCDebug(cadCommands) << "Unregistering command:" << name;
    m_commands.remove(name.toLower());
}

void CommandManager::unregisterAlias(const QString& alias)
{
    qCDebug(cadCommands) << "Unregistering alias:" << alias;
    m_aliases.remove(alias.toLower());
}

QStringList CommandManager::getAvailableCommands() const
{
    QStringList commands;
    for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
        commands.append(it.key());
    }
    commands.sort();
    return commands;
}

QStringList CommandManager::getCommandCompletions(const QString& partial) const
{
    QStringList completions;
    QString lowerPartial = partial.toLower();
    
    for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
        if (it.key().startsWith(lowerPartial)) {
            completions.append(it.key());
        }
    }
    
    for (auto it = m_aliases.begin(); it != m_aliases.end(); ++it) {
        if (it.key().startsWith(lowerPartial)) {
            completions.append(it.key());
        }
    }
    
    completions.sort();
    return completions;
}

QString CommandManager::getCommandHelp(const QString& command) const
{
    QString lowerCommand = command.toLower();
    auto it = m_commands.find(lowerCommand);
    if (it != m_commands.end()) {
        return it->help;
    }

    // Check if it's an alias
    auto aliasIt = m_aliases.find(lowerCommand);
    if (aliasIt != m_aliases.end()) {
        auto commandIt = m_commands.find(aliasIt.value());
        if (commandIt != m_commands.end()) {
            return QString("Alias for %1: %2").arg(aliasIt.value(), commandIt->help);
        }
    }

    return QString("No help available for command: %1").arg(command);
}

bool CommandManager::executeScript(const QString& scriptPath)
{
    QFileInfo fileInfo(scriptPath);
    if (!fileInfo.exists()) {
        qCWarning(cadCommands) << "Script file does not exist:" << scriptPath;
        return false;
    }

    QFile file(scriptPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(cadCommands) << "Cannot open script file:" << scriptPath;
        return false;
    }

    QTextStream in(&file);
    QString scriptText = in.readAll();
    file.close();

    qCDebug(cadCommands) << "Executing script:" << scriptPath;
    return executeScriptText(scriptText);
}

bool CommandManager::executeScriptText(const QString& scriptText)
{
    QStringList lines = scriptText.split('\n', Qt::SkipEmptyParts);
    bool success = true;

    beginGroup("Script Execution");

    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();

        // Skip comments and empty lines
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(';') || trimmedLine.startsWith('#')) {
            continue;
        }

        if (!executeCommand(trimmedLine)) {
            success = false;
            qCWarning(cadCommands) << "Script command failed:" << trimmedLine;
        }
    }

    endGroup();
    return success;
}

void CommandManager::startRecording(const QString& macroName)
{
    if (m_recording) {
        qCWarning(cadCommands) << "Already recording macro:" << m_currentMacroName;
        stopRecording();
    }

    qCDebug(cadCommands) << "Starting macro recording:" << macroName;
    m_recording = true;
    m_currentMacroName = macroName;
    m_recordedCommands.clear();
}

void CommandManager::stopRecording()
{
    if (!m_recording) {
        qCWarning(cadCommands) << "Not currently recording";
        return;
    }

    qCDebug(cadCommands) << "Stopping macro recording:" << m_currentMacroName;

    if (!m_recordedCommands.isEmpty()) {
        m_macros[m_currentMacroName] = m_recordedCommands;

        // Save macro to file
        QString macroPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/macros";
        QDir().mkpath(macroPath);

        QFile file(macroPath + "/" + m_currentMacroName + ".scr");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (const QString& command : m_recordedCommands) {
                out << command << "\n";
            }
            file.close();
        }
    }

    m_recording = false;
    m_currentMacroName.clear();
    m_recordedCommands.clear();
}

void CommandManager::playMacro(const QString& macroName)
{
    auto it = m_macros.find(macroName);
    if (it != m_macros.end()) {
        qCDebug(cadCommands) << "Playing macro:" << macroName;

        beginGroup(QString("Macro: %1").arg(macroName));
        for (const QString& command : it.value()) {
            executeCommand(command);
        }
        endGroup();
    } else {
        // Try to load from file
        QString macroPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/macros/" + macroName + ".scr";
        if (QFileInfo::exists(macroPath)) {
            executeScript(macroPath);
        } else {
            qCWarning(cadCommands) << "Macro not found:" << macroName;
        }
    }
}

QStringList CommandManager::getAvailableMacros() const
{
    QStringList macros;

    // Add loaded macros
    for (auto it = m_macros.begin(); it != m_macros.end(); ++it) {
        macros.append(it.key());
    }

    // Add file-based macros
    QString macroPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/macros";
    QDir macroDir(macroPath);
    QStringList scriptFiles = macroDir.entryList(QStringList() << "*.scr", QDir::Files);

    for (const QString& file : scriptFiles) {
        QString macroName = QFileInfo(file).baseName();
        if (!macros.contains(macroName)) {
            macros.append(macroName);
        }
    }

    macros.sort();
    return macros;
}

void CommandManager::setUndoLimit(int limit)
{
    m_undoLimit = qMax(1, limit);
    trimHistory();
}

void CommandManager::repeatLastCommand()
{
    if (m_lastCommand.isEmpty()) {
        qCWarning(cadCommands) << "No previous command to repeat";
        return;
    }

    qCDebug(cadCommands) << "Repeating last command:" << m_lastCommand;
    executeCommand(m_lastCommand);
}

void CommandManager::cancelCurrentCommand()
{
    if (m_commandInProgress) {
        qCDebug(cadCommands) << "Cancelling current command";
        m_commandInProgress = false;
        // TODO: Implement command cancellation mechanism
    }
}

void CommandManager::onCommandCompleted()
{
    m_commandInProgress = false;
}

void CommandManager::onCommandFailed(const QString& error)
{
    m_commandInProgress = false;
    qCWarning(cadCommands) << "Command failed:" << error;
}

// Private methods
void CommandManager::addToHistory(std::unique_ptr<CADCommand> command)
{
    if (!command || !command->canUndo()) {
        return;
    }

    // Clear redo stack when adding new command
    m_redoStack.clear();

    // Add to undo stack
    m_undoStack.push(std::move(command));

    // Trim history if needed
    trimHistory();

    updateUndoRedoState();
    emit historyChanged();
}

void CommandManager::trimHistory()
{
    while (m_undoStack.size() > m_undoLimit) {
        m_undoStack.removeFirst();
    }
}

void CommandManager::updateUndoRedoState()
{
    bool canUndoNow = canUndo();
    bool canRedoNow = canRedo();

    emit undoAvailabilityChanged(canUndoNow);
    emit redoAvailabilityChanged(canRedoNow);
}

QStringList CommandManager::parseCommandLine(const QString& commandLine)
{
    QStringList parts;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < commandLine.length(); ++i) {
        QChar c = commandLine[i];

        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c.isSpace() && !inQuotes) {
            if (!current.isEmpty()) {
                parts.append(current);
                current.clear();
            }
        } else {
            current.append(c);
        }
    }

    if (!current.isEmpty()) {
        parts.append(current);
    }

    return parts;
}

QString CommandManager::resolveAlias(const QString& command)
{
    auto it = m_aliases.find(command.toLower());
    if (it != m_aliases.end()) {
        return it.value();
    }
    return command.toLower();
}

std::unique_ptr<CADCommand> CommandManager::createCommand(const QString& name, const QStringList& args)
{
    auto it = m_commands.find(name.toLower());
    if (it != m_commands.end()) {
        try {
            return it->factory(args);
        } catch (const std::exception& e) {
            qCWarning(cadCommands) << "Failed to create command" << name << ":" << e.what();
            return nullptr;
        }
    }

    return nullptr;
}

void CommandManager::initializeBuiltinCommands()
{
    qCDebug(cadCommands) << "Initializing builtin commands...";

    // Basic commands
    registerBuiltinCommand("line", "Draw a line", [](const QStringList& args) -> std::unique_ptr<CADCommand> {
        // TODO: Implement LineCommand
        return nullptr;
    });

    registerBuiltinCommand("circle", "Draw a circle", [](const QStringList& args) -> std::unique_ptr<CADCommand> {
        // TODO: Implement CircleCommand
        return nullptr;
    });

    registerBuiltinCommand("rectangle", "Draw a rectangle", [](const QStringList& args) -> std::unique_ptr<CADCommand> {
        // TODO: Implement RectangleCommand
        return nullptr;
    });

    // Common aliases
    registerAlias("l", "line");
    registerAlias("c", "circle");
    registerAlias("rec", "rectangle");
    registerAlias("rect", "rectangle");
    registerAlias("u", "undo");
    registerAlias("z", "zoom");
    registerAlias("p", "pan");

    qCDebug(cadCommands) << "Builtin commands initialized";
}

void CommandManager::registerBuiltinCommand(const QString& name, const QString& help,
                                          std::function<std::unique_ptr<CADCommand>(const QStringList&)> factory)
{
    CommandInfo info;
    info.name = name;
    info.help = help;
    info.factory = factory;

    m_commands[name.toLower()] = info;
}
