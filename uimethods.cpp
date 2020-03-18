/*
User interaction methods.
*/

const string VERSION("joblog version 0.0.1");

const string HELPMSG(
  "Useage: joblog [--version] [--help] [-<args>] <command> [<command args>]\n"
  "\n"
  "Commands:\n"
  "  help    Print this help message or further help on a topic.\n"
  "  init    Initialize a logfile.\n"
  "  start   Begin working.\n"
  "  end     End working.\n"
  "  log     Write down what you did.\n"
  "  state   Give a short overview of the current state.\n"
  "  list    List what was done.\n"
  "\n"
  "Use 'joblog help <topic>' to get further help on a topic.\n"
  "Available topics are: start, end, list, args"
);

const string HELPMSG_START(
    "joblog start [-a]\n"
    "\n"
    "Call this when you start working.\n"
    "Arguments:\n"
    " -a  When you called start already and want to correct this by moving\n"
    "     the start to the current time.\n"
);

const string HELPMSG_END(
    "joblog end [-a]\n"
    "\n"
    "Call this when you are about to end working for now.\n"
    "Arguments:\n"
    " -a  When you called end already and want to correct this by moving\n"
    "     the end to the current time.\n"
);

const string HELPMSG_LIST(
  "joblog list [-s] [<specifier>]\n"
  "\n"
  "List the recent work. The time specifier can be:\n"
  " 1) Empty. Work of this day will be listed.\n"
  " 2) One of the following characters:\n"
  //TODO "     'a' - this session\n"
  "     'd' - Today\n"
  "     'w' - This week (since Monday morning)\n"
  "     'm' - This month (since the 1st)\n"
  " 3) <amount><unit> where amount is an integer and unit is\n"
  "     'm' - Minutes \n"
  "     'h' - Hours \n"
  "     'd' - Days \n"
  " 3) A single date in the form 'dd.mm.yyyy'. Work after this date will\n"
  "    be listed.\n"
  " 4) Two dates in that form, separated by a minus:\n"
  "    'dd.mm.yyyy - dd.mm.yyyy'. Work between these days will be listed.\n"
  "\n"
  "Arguments:\n"
  " -s  Do not list log notes."
);

const string HELPMSG_ARGS(
    "Available arguments are:\n"
    " -path=<path>   Specify to use a given path instead of searching for\n"
    "                  default path. Do not end with '/'.\n"
    " -c             Check the integrity of the file."
);

bool getLoglist(Joblog *joblog, LogList **out) {
    try {
        *out = joblog->getLogList();
    } catch (CorruptedFileException& ex) {
        std::cout << "The logfile is corrupted. Try to fix it manually.\n"
                     "The exeptions message is:\n"
                     "  '" << ex.what() << "'" << std::endl;
        return false;
    }
    return true;
}

int list(LogList *loglist, vector<string> args) {
    // default settings
    bool listLogs=true;
    dt::time_point from = dt::now();
    dt::time_point to = dt::now();
    
    // parse arguments
    while( args.size() > 0 && (args[0][0] == '-')) {
        if (args[0].compare("-s") == 0) {
            listLogs = false;
        }
        else {
            std::cout << "Unkown option." << std::endl;
            return 1;
        }
        args.erase(args.begin());
    }
    
    // parse time specifier
    bool success = false;
    if (args.size() == 0) {
        from = dt::getBeginOfDay(from);
        success = true;
    }
    else if (args.size() == 1) {
        if (args[0].compare("d") == 0) {
            from = dt::getBeginOfDay(from);
            success = true;
        }
        else if (args[0].compare("w") == 0) {
            from = dt::getBeginOfDay(dt::getLastMonday(from));
            success = true;
        }
        else if (args[0].compare("m") == 0) {
            from = dt::getBeginOfDay(dt::getLastFirstOfMonth(from));
            success = true;
        }
        try {
            dt::duration d = dt::parseDurationStr(args[0]);
            from -= d;
            success = true;
        } catch(dt::DateFormatException& ex) {}
        try {
            from = dt::parseDateStr(args[0]);
            success = true;
        } catch(dt::DateFormatException& ex) {}
    }
    else if (args.size() == 3) {
        // remove delimiter between dates
        args.erase(args.begin() + 1);
    }
    if (args.size() == 2) {
        try {
            from = dt::parseDateStr(args[0]);
            to = dt::parseDateStr(args[1]);
            success = true;
        } catch(dt::DateFormatException& ex) {}
    }
    if (! success) {
        std::cout << "Unkown date specifier. ";
        std::cout << "Use 'help list' for help." << std::endl;
        return 2;
    }
    
    // print information
    vector<LogEntry *> allentries = loglist->list(from, to, listLogs);
    
    dt::time_point last_start = to;
    dt::duration workedtime = dt::seconds(0);
    vector<LogEntryLog *> notes;
    for (vector<LogEntry *>::iterator e = allentries.begin();
           e != allentries.end(); ++e) {
        if ((*e)->type() == LogEntryType::start) {
            last_start = (*e)->getTime();
        }
        else if ((*e)->type() == LogEntryType::log) {
            notes.push_back((LogEntryLog *) *e);
        }
        else if ((*e)->type() == LogEntryType::end) {
            dt::duration thistime = (*e)->getTime() - last_start;
            std::cout << dt::toDateString(last_start) << ": Worked ";
            std::cout << dt::toString(thistime) << std::endl;
            for (LogEntryLog *e : notes) {
                std::cout << " - " << e->getNote() << std::endl;
            }
            workedtime += thistime;
            notes.clear();
        }
        else dbglg("wtf"); // should really not happen
    }
    std::cout << "\nOverall: " << dt::toString(workedtime) << std::endl;
    return 0;
}

int parseNormalCommand(Joblog* joblog, std::vector<string> args) {
    if (args[0].compare("help") == 0) {
        if (args.size() < 2) {
            std::cout << HELPMSG << std::endl;
            return 0;
        }
        else {
            if (args[1].compare("start") == 0) {
                std::cout << HELPMSG_START << std::endl;
                return 0;
            }
            if (args[1].compare("end") == 0) {
                std::cout << HELPMSG_END << std::endl;
                return 0;
            }
            if (args[1].compare("list") == 0) {
                std::cout << HELPMSG_LIST << std::endl;
                return 0;
            }
            else if (args[1].compare("args") == 0) {
                std::cout << HELPMSG_ARGS << std::endl;
                return 0;
            }
            else {
                std::cout << "No help on this topic available." << std::endl;
                return 2;
            }
        }
    }
    if (args[0].compare("init") == 0) {
        try {
            return joblog->init();
        } catch (CorruptedFileException& ex) {
            std::cout << "Init failed. The exception message is:\n"
                         "'" << ex.what() << "'\n"
                         "Note that this could mean this folder is already "
                         "initialized" << std::endl;
            return 2;
        }
    }
    if (args[0].compare("start") == 0) {
        LogList *loglist;
        if (! getLoglist(joblog, &loglist)) return 2;
        
        bool again = false;
        if (args.size() > 1 && args[1].compare("-a") == 0)
            again = true;
        
        try {
            loglist->start(again);
        } catch (SituationalMistake& ex) {
            if (loglist->isActive())
                std::cout << "Already started.\nIf you want to move the start "
                          << "to now, use 'start -a'." << std::endl;
                //TODO suggest forgotten end
            else
                std::cout << "Cannot start again when something "
                          << "happend in between." << std::endl;
            return 2;
        }
        std::cout << "Started at "
                  << dt::toClockTimeStr(loglist->getLastEntry()->getTime())
                  << "." << std::endl;
        return 0;
    }
    if (args[0].compare("end") == 0) {
        LogList *loglist;
        if (! getLoglist(joblog, &loglist)) return 2;
        
        bool again = false;
        if (args.size() > 1 && args[1].compare("-a") == 0)
            again = true;
        
        try {
            loglist->end(again);
        } catch (SituationalMistake& ex) {
            if (loglist->isActive())
                dbglg("Weird case, coming later...");
            else
                std::cout << "You need to start first." << std::endl;
                //TODO suggest start previously
            return 2;
        }
        dt::duration worked = loglist->getLastEntry()->getTime() -
                                  loglist->getLastStart()->getTime();
        std::cout << "End noted. You worked " << dt::toString(worked)
                  << "." << std::endl;
        return 0;
    }
    if (args[0].compare("log") == 0) {
        if (args.size() < 2) {
            std::cout << "Empty log discarded." << std::endl;
            return 2;
        }
        LogList *loglist;
        if (! getLoglist(joblog, &loglist)) return 2;
        try {
            std::stringstream note;
            note << args[1];
            for (int i=2; i<args.size(); i++)
                note << " " << args[i];
            loglist->log(note.str());
        } catch (SituationalMistake& ex) {
            std::cout << "You need to start before writing logs." << std::endl;
            return 2;
        }
        std::cout << "Log noted." << std::endl;
        return 0;
    }
    if (args[0].compare("state") == 0) {
        LogList *loglist;
        if (! getLoglist(joblog, &loglist)) return 2;
        if (!loglist->isActive()) {
            std::cout << "Not working." << std::endl;
            return 0;
        }
        else {
            dt::duration worked = dt::now()-loglist->getLastStart()->getTime();
            std::cout << "Worked " << dt::toString(worked) << "." << std::endl;
            return 0;
        }
    }
    if (args[0].compare("list") == 0) {
        LogList *loglist;
        if (! getLoglist(joblog, &loglist)) return 2;
        args.erase(args.begin());
        return list(loglist, args);
    }
    
    std::cout << "Unknown command '" << args[0] <<
                 "'. Use --help to see usage." << std::endl;
    return 2;
}

int interactiveMode(Joblog* joblog) {
    dbglg("started interactive mode");
    //TODO
    // for (command << std::cin; command.compare("exit") != 0; ) {
    //      ...
    // }
    return 0;
}

int commandLineInterface(vector<string> args) {
    // Test for help or version arguments
    if (args.size()>0 && args[0].compare("--help") == 0) {
        std::cout << HELPMSG << std::endl;
        return 0;
    }
    if (args.size()>0 && args[0].compare("--version") == 0) {
        std::cout << VERSION << std::endl;
        return 0;
    }
    
    // Give all the arguments to the Builder.
    // Arguments are what starts with a minus.
    Joblog *joblog = new Joblog();
    while (args.size() > 0 && args[0][0]=='-') {
        if (args[0].compare(0, 6, "-path=") == 0) {
            joblog->setPath(args[0].substr(6));
        }
        else if (args[0].compare("-c") == 0) {
            joblog->doChecks();
        }
        else {
            std::cout << "Unknown argument '" << args[0] << "'" << std::endl;
            std::cout << "Use --help to see valid commands." << std::endl;
            delete joblog;
            return 2;
        }
        args.erase(args.begin());
    }
    
    int res;
    
    // If there is no command after the arguments, enter interactive mode
    if (args.size() == 0) {
        res = interactiveMode(joblog);
    }
    // Else, parse the command
    else {
        res = parseNormalCommand(joblog, args);
    }
    
    joblog->save();
    delete joblog;
    return res;
};
