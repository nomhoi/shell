#include <iostream>
#include <list>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>

#include <algorithm>
#include <cctype>
#include <locale>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

using namespace std;

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

const vector<string> split(const string& line, const char& delimeter = ' ')
{
    vector<string> tokens;
    stringstream stream(line);
    string token;
    while(getline(stream, token, delimeter)) {
        if (token.size() > 0) {
            trim(token);
            tokens.push_back(token);
        }
    }
    return tokens;
}

int main (int argc, char ** argv) {
    char buf[PIPE_BUF];
    memset(buf, 0, PIPE_BUF);
    read(STDIN_FILENO, buf, PIPE_BUF);
    //cerr << "buf: "<< buf << endl;
    string in(buf);
    vector<string> commands = split(in, '|');

    string text;
    for (vector<string>::const_iterator it = commands.begin(); it != commands.end(); ++it) {
        string command = *it;
        //cerr << "command: " << command << endl;

        int pipe_to_master[2];
        pipe(pipe_to_master);
        int pipe_to_puppet[2];
        pipe(pipe_to_puppet);

        pid_t pid = fork();
        if (pid == -1) {
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (it != commands.begin()) {
                close(STDIN_FILENO);
                dup2(pipe_to_puppet[0], STDIN_FILENO);
                close(pipe_to_puppet[1]);
                close(pipe_to_puppet[0]);
            }

            close(STDOUT_FILENO);
            dup2(pipe_to_master[1], STDOUT_FILENO);
            close(pipe_to_master[1]);
            close(pipe_to_master[0]);

//            dup2(STDERR_FILENO, STDOUT_FILENO);
//            close(STDERR_FILENO);

//            dup2(STDOUT_FILENO, STDERR_FILENO);
//            close(STDERR_FILENO);

            vector<string> cmd = split(command);
            char *argv[cmd.size() + 1];
            for (unsigned int i = 0; i < cmd.size(); i++) {
                argv[i] = (char *)cmd[i].c_str();
                //cerr << "argv[" << i << "]: "<< argv[i] << endl;
            }
            argv[cmd.size()] = NULL;

            execvp(argv[0], argv);
        } else {
            if (it != commands.begin()) {
                write(pipe_to_puppet[1], text.c_str(), text.size());
                close(pipe_to_puppet[1]);
                close(pipe_to_puppet[0]);
            }

            close(STDIN_FILENO);
            dup2(pipe_to_master[0], STDIN_FILENO);
            close(pipe_to_master[1]);
            close(pipe_to_master[0]);

            char buf[PIPE_BUF];
            memset(buf, 0, PIPE_BUF);
            read (0, buf, PIPE_BUF);
            //cerr << "master buf:\n"<< buf << endl;

            text = string(buf);
        }
    }

    ofstream out("/home/box/result.out");
    out << text;
    out.close();

    return 0;
}
