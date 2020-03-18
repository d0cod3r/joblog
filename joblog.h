/* Class definitions for joblog.cpp
 */

// -----------------------------------------------------------------------------
//  Exceptions
// -----------------------------------------------------------------------------

class CustomException : public std::exception {
protected:
    string msg;
public:
    CustomException(const string& msg) {
        this->msg = msg;
    }
    virtual const char* what() {
        return this->msg.c_str();
    }
};

/* This exception is thrown when there is something wrong in the saved files
 * or when files are missing. */
class CorruptedFileException : public CustomException {
public:
    CorruptedFileException(const string& msg) :
        CustomException(msg) {};
};

/* This exception is thrown when a given command is not expected due to the
 * status obtained from the file. */
class SituationalMistake : public CustomException {
public:
    SituationalMistake(const string& msg) :
        CustomException(msg) {};
};


// -----------------------------------------------------------------------------
//  LogEntry and subclasses
// -----------------------------------------------------------------------------

enum class LogEntryType {
    start, end, log
};

/* This is the superclass to all entries stored in the logfile. */
class LogEntry {
private:
    dt::time_point time;
protected:
    LogEntry();
    LogEntry(const dt::time_point&);
public:
    static LogEntry * parse(const string&);
    virtual string toString();
    virtual LogEntryType type() = 0;
    dt::time_point getTime();
    virtual ~LogEntry() = default;
};

class LogEntryStart : public LogEntry {
public:
    LogEntryStart() : LogEntry() {};
    LogEntryStart(const dt::time_point& time) : LogEntry(time) {};
    virtual LogEntryType type() { return LogEntryType::start; };
    virtual string toString();
};

class LogEntryEnd : public LogEntry {
public:
    LogEntryEnd() : LogEntry() {};
    LogEntryEnd(const dt::time_point& time) : LogEntry(time) {};
    virtual LogEntryType type() { return LogEntryType::end; };
    virtual string toString();
};

class LogEntryLog : public LogEntry {
private:
    string note;
public:
    LogEntryLog(const string&);
    LogEntryLog(const dt::time_point&, const string&);
    virtual LogEntryType type() { return LogEntryType::log; };
    string getNote();
    virtual string toString();
};


// -----------------------------------------------------------------------------
//  Main Content Objects
// -----------------------------------------------------------------------------

/* This class is associated with the file 'logs' and stores the list of events.
 * It offers tools to add and list events. */
class LogList {
private:
    std::fstream *file;
    int needsToBeWritten;
    vector<LogEntry *> entries;
    bool active;
protected:
    void updateFileState();
public:
    LogList(std::fstream *);
    ~LogList();
    bool isActive();
    void check();
    void save();
    void start(bool);
    void log(string);
    void end(bool);
    LogEntry *getLastEntry();
    LogEntryStart *getLastStart();
    vector<LogEntry *> list(dt::time_point&, dt::time_point&, bool&);
};

/* This class stores information about the job, e.g. how many hours should be
 * worked in a week. */
class JobProperties {
private:
    std::fstream *file;
    float weeklyhours;
public:
    JobProperties(std::fstream *);
    void save();
};

/* This is the main class of this program. It stores pointers to the content
 * classes. */
class Joblog {
private:
    string path;
    bool check;
    LogList *loglist;
    JobProperties *jobproperties;
protected:
    void loadLoglist();
    void loadProperties();
public:
    Joblog();
    ~Joblog();
    void setPath(string);
    int init();
    void doChecks();
    void save();
    LogList *getLogList();
};
