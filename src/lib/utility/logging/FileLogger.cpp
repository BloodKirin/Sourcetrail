#include "FileLogger.h"

#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>

#include "utility/file/FileSystem.h"
#include "utility/utilityString.h"

std::wstring FileLogger::generateDatedFileName(const std::wstring& prefix, const std::wstring& suffix)
{
	time_t time;
	std::time(&time);
	tm t = *std::localtime(&time);

	std::wstringstream filename;
	if (!prefix.empty())
	{
		filename << prefix;
		filename << L"_";
	}
	filename << t.tm_year + 1900 << L"-";
	filename << (t.tm_mon < 9 ? L"0" : L"") << t.tm_mon + 1 << L"-";
	filename << (t.tm_mday < 10 ? L"0" : L"") << t.tm_mday << L"_";
	filename << (t.tm_hour < 10 ? L"0" : L"") << t.tm_hour << L"-";
	filename << (t.tm_min < 10 ? L"0" : L"") << t.tm_min << L"-";
	filename << (t.tm_sec < 10 ? L"0" : L"") << t.tm_sec;

	if (!suffix.empty())
	{
		filename << L"_";
		filename << suffix;
	}

	return filename.str();
}

FileLogger::FileLogger()
	: Logger("FileLogger")
	, m_logFileName(L"log")
	, m_logDirectory(L"user/log/")
	, m_maxLogLineCount(0)
	, m_maxLogFileCount(0)
	, m_currentLogLineCount(0)
	, m_currentLogFileCount(0)
{
	updateLogFileName();
}

FileLogger::~FileLogger()
{
}

FilePath FileLogger::getLogFilePath() const
{
	return m_currentLogFilePath;
}

void FileLogger::setLogFilePath(const FilePath& filePath)
{
	m_currentLogFilePath = filePath;
	m_logFileName = L"";
}

void FileLogger::setLogDirectory(const FilePath& filePath)
{
	m_logDirectory = filePath;
	FileSystem::createDirectory(m_logDirectory);
}

void FileLogger::setFileName(const std::wstring& fileName)
{
	if (fileName != m_logFileName)
	{
		m_logFileName = fileName;
		m_currentLogLineCount = 0;
		m_currentLogFileCount = 0;
		updateLogFileName();
	}
}

void FileLogger::logInfo(const LogMessage& message)
{
	logMessage("INFO", message);
}

void FileLogger::logWarning(const LogMessage& message)
{
	logMessage("WARNING", message);
}

void FileLogger::logError(const LogMessage& message)
{
	logMessage("ERROR", message);
}

void FileLogger::setMaxLogLineCount(unsigned int lineCount)
{
	m_maxLogLineCount = lineCount;
}

void FileLogger::setMaxLogFileCount(unsigned int fileCount)
{
	m_maxLogFileCount = fileCount;
}

void FileLogger::updateLogFileName()
{
	if (m_logFileName.empty())
	{
		return;
	}

	bool fileChanged = false;

	std::wstring currentLogFilePath = m_logDirectory.wstr() + m_logFileName;
	if (m_maxLogFileCount > 0)
	{
		currentLogFilePath += L"_";
		if (m_currentLogLineCount >= m_maxLogLineCount)
		{
			m_currentLogLineCount = 0;

			m_currentLogFileCount++;
			if (m_currentLogFileCount >= m_maxLogFileCount)
			{
				m_currentLogFileCount = 0;
			}
			fileChanged = true;
		}
		currentLogFilePath += std::to_wstring(m_currentLogFileCount);

	}
	currentLogFilePath += L".txt";

	m_currentLogFilePath = FilePath(currentLogFilePath);

	if (fileChanged)
	{
		FileSystem::remove(m_currentLogFilePath);
	}
}

void FileLogger::logMessage(const std::string& type, const LogMessage& message)
{
	std::ofstream fileStream;
	fileStream.open(m_currentLogFilePath.str(), std::ios::app);
	fileStream << message.getTimeString("%H:%M:%S") << " | ";
	fileStream << message.threadId << " | ";

	if (message.filePath.size())
	{
		fileStream << message.getFileName() << ':' << message.line << ' ' << message.functionName << "() | ";
	}

	fileStream << type << ": " << utility::encodeToUtf8(message.message) << std::endl;
	fileStream.close();

	m_currentLogLineCount++;
	if (m_maxLogFileCount > 0)
	{
		updateLogFileName();
	}
}
