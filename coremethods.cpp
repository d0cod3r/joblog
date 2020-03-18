/* Core methods
 */

/* Create a LogEntry for a recent event. */
LogEntry::LogEntry() {
    this->time = dt::now();
}

/* Create a LogEntry for a specified time. */
LogEntry::LogEntry(const dt::time_point& time) {
    this->time = time;
}

/* Create a string representation to be saved in a file. */
string LogEntry::toString() {
    return dt::toString(this->time) + " ";
}

dt::time_point LogEntry::getTime() {
    return this->time;
}

LogEntryLog::LogEntryLog(const string& note)
  : LogEntry() {
    this->note = note;
}

LogEntryLog::LogEntryLog(const dt::time_point& time, const string& note)
  : LogEntry(time) {
    this->note = note;
}

string LogEntryLog::getNote() {
    return this->note;
}

/* Parse one line of a file and recreate the LogEntry. */
LogEntry * LogEntry::parse(const string& str) {
    // The format is 'dd.mm.YYYY hh:mm:ss <command> <args...>'
    dt::time_point time;
    try {
        time = dt::parseDateStr(str.substr(0, dt::DATESIZE));
    } catch(dt::DateFormatException& ex) {
        throw CorruptedFileException(
            "Could not parse date "+str.substr(0, dt::DATESIZE));
    }
    if (str.size() < dt::DATESIZE + 1) {
        throw CorruptedFileException("Empty line after date");
    }
    string content = str.substr(dt::DATESIZE + 1);
    if (content.compare("start") == 0) {
        return new LogEntryStart(time);
    }
    else if (content.compare("end") == 0) {
        return new LogEntryEnd(time);
    }
    else if (content.compare(0,4,"log ") == 0) {
        string msg = content.substr(4);
        return new LogEntryLog(time, msg);
    }
    else {
        throw CorruptedFileException("Unknown log entry "+content);
    }
}

string LogEntryStart::toString() {
    string res = LogEntry::toString();
    return res + "start";
}

string LogEntryEnd::toString() {
    string res = LogEntry::toString();
    return res + "end";
}

string LogEntryLog::toString() {
    string res = LogEntry::toString();
    return res + "log " + this->note;
}


/* Parse the log file. */
LogList::LogList(std::fstream *filestream) {
    this->needsToBeWritten = 0;
    this->active = false;
    this->file = filestream;
    
    string line;
    while(std::getline(*filestream, line) && !line.empty()) {
        LogEntry *newEntry = LogEntry::parse(line);
        if (newEntry->type() == LogEntryType::start) {
            this->active = true;
        }
        else if (newEntry->type() == LogEntryType::end) {
            this->active = false;
        }
        this->entries.push_back(newEntry);
    }
}

/* Perform checks on the logfile. */
void LogList::check() {
    bool active = false;
    for (LogEntry *entry : this->entries) {
        if (entry->type() == LogEntryType::start) {
            if (active)
                throw CorruptedFileException("Two starts without end");
            active = true;
        }
        else if (entry->type() == LogEntryType::end) {
            if (! active)
                throw CorruptedFileException("Two ends without start");
            active = false;
        }
    }
    for (int i=0; i<this->entries.size()-1; i++) {
        if ( this->entries[i]->getTime() > this->entries[i+1]->getTime() ) {
            throw CorruptedFileException("Entries not sorted");
        }
    }
}

/* If an entry is added, it can be appended to the file. If the file has to be
 * rewritten, keep it that way. */
void LogList::updateFileState() {
    if (this->needsToBeWritten != -1)
        this->needsToBeWritten += 1;
}

bool LogList::isActive() {
    return this->active;
}

void LogList::start(bool again) {
    if (!this->active) {
        this->entries.push_back( new LogEntryStart() );
        this->active = true;
        this->updateFileState();
    }
    else if (!again) {
        throw SituationalMistake("Already started");
    }
    else if (this->getLastEntry()->type() != LogEntryType::start) {
        throw SituationalMistake(
                "Cannot move start if something was noted in between." );
    }
    else {
        LogEntry *oldstart = this->entries.back();
        this->entries.pop_back();
        delete oldstart;
        this->entries.push_back( new LogEntryStart() );
        this->needsToBeWritten = -1;
    }
}

void LogList::log(string note) {
    if (! this->active)
        throw SituationalMistake("Log is only enabled during work");
    this->entries.push_back(
        new LogEntryLog(note) );
    this->updateFileState();
}

void LogList::end(bool again) {
    if (this->active) {
        this->entries.push_back( new LogEntryEnd() );
        this->active = false;
        this->updateFileState();
    }
    else if (!again) {
        throw SituationalMistake("Not started");
    }
    else {
        LogEntry *oldend = this->entries.back();
        this->entries.pop_back();
        delete oldend;
        this->entries.push_back( new LogEntryEnd() );
        this->needsToBeWritten = -1;
    }
}

/* Pick out the entries between the given dates. */
vector<LogEntry *> LogList::list(dt::time_point& from, dt::time_point& to,
                                      bool& includeLogs) {
    vector<LogEntry *> res;
    for (LogEntry *e : this->entries) {
        if ((e->getTime() > from) && (e->getTime() < to)) {
            if (e->type() == LogEntryType::start ||
                                e->type() == LogEntryType::end)
                res.push_back(e);
            if (e->type() == LogEntryType::log && includeLogs)
                    res.push_back(e);
        }
    }
    return res;
}

LogEntry * LogList::getLastEntry() {
    return this->entries.back();
}

LogEntryStart * LogList::getLastStart() {
    vector<LogEntry *>::reverse_iterator res;
    res = this->entries.rbegin();
    while (res != this->entries.rend()) {
        if ((*res)->type() == LogEntryType::start) {
            return (LogEntryStart *) (*res);
        }
        ++res;
    }
    throw SituationalMistake("No start found");
}

/* Write this object to the file it was created from. */
void LogList::save() {
    if (this->needsToBeWritten == -1) {
        // rewrite all
        this->file->clear();
        this->file->seekp(0);
        for (LogEntry *entry : this->entries) {
            (*this->file) << entry->toString() << std::endl;
        }
    }
    else {
        // append last logs
        this->file->clear();
        this->file->seekp(0, std::ios_base::end);
        for (int pos = this->entries.size()-this->needsToBeWritten;
                                   pos < this->entries.size(); pos++) {
            (*this->file) << this->entries[pos]->toString() << std::endl;
        }
    }
}

LogList::~LogList() {
    this->file->flush();
    this->file->close();
    delete this->file;
    
    for (LogEntry *entry : this->entries) {
        delete entry;
    }
    this->entries.clear();
}

/* Search the path and read in the list of logs. */
void Joblog::loadLoglist() {
    if (this->loglist) {
        // If the Loglist was already loaded, return
        return;
    }
    std::fstream *filestream = new std::fstream();
    // If a path was specified, use that one
    if (! this->path.empty()) {
        filestream->open(this->path + "/logs", FILEMODE);
    }
    // Else, search for the default file in parent directories
    else {
        string currentFolder = "";
        string filename = SAVEPATH + "/logs";
        filestream->open(filename);
        for (int i=1; i<SEARCHDEPTH && !(*filestream); i++) {
            currentFolder += "../";
            filestream->open(currentFolder + filename, FILEMODE);
        }
        this->path = currentFolder;
    }
    // Test the file
    if (! filestream->good())
        throw CorruptedFileException("Could not open a logs file");
    
    this->loglist = new LogList(filestream);
    
    if (this->check) {
        loglist->check();
    }
}

Joblog::Joblog() {
    this->path.clear();
    this->check = false;
    this->loglist = nullptr;
}

void Joblog::setPath(string path) {
    this->path = path;
}

/* Create a new directory and the necessary files in it. */
int Joblog::init() {
    if (this->path.empty()) {
        this->path = SAVEPATH;
    }
    int res = mkdir((this->path).c_str(), 0);
    if (res != 0) {
        throw CorruptedFileException("Could not create a directory.");
    }
    string logfilename = this->path + "/" + "logs";
    std::ofstream file(logfilename);
    if (! file.good()) {
        throw CorruptedFileException("Could not create a log file.");
    }
    return 0;
}

/* Enforce checking all files that will be used. */
void Joblog::doChecks() {
    this->check = true;
}

/* Save all used objects. */
void Joblog::save() {
    if (this->loglist) {
        this->loglist->save();
    }
}

LogList *Joblog::getLogList() {
    this->loadLoglist();
    return this->loglist;
}

Joblog::~Joblog() {
    if (this->loglist) {
        delete this->loglist;
    }
}
