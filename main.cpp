/** https://github.com/Will-Eves/HexTools By William Eves / Devparty **/
#include "HexTools.h"

/** https://github.com/jupyter-xeus/cpp-terminal By Jupyter Xeus **/ 
#include "../vendor/cppterminal/base.hpp"
#include "../vendor/cppterminal/input.hpp"

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

// Hex Data

HexTools::HexData hexData;

/*** defines ***/

#define KILO_VERSION "0.1"
#define KILO_TAB_STOP 4
#define KILO_QUIT_TIMES 3

using Term::color;
using Term::cursor_off;
using Term::cursor_on;
using Term::erase_to_eol;
using Term::fg;
using Term::Key;
using Term::move_cursor;
using Term::style;
using Term::Terminal;

enum dhexHighlight {
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

/*** data ***/

struct dhexSyntax {
    const char* filetype;
    const char** filematch;
    const char** keywords;
    const char* singleline_comment_start;
    const char* multiline_comment_start;
    const char* multiline_comment_end;
    int flags;
};

typedef struct erow {
    int idx;
    int size;
    int rsize;
    char* chars;
    char* render;
    unsigned char* hl;
    int hl_open_comment;
} erow;

struct dhexConfig {
    int cx, cy;
    int rx;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    int numrows;
    erow* row;
    int dirty;
    char* filename;
    char statusmsg[80];
    time_t statusmsg_time;
    struct dhexSyntax* syntax;
};

struct dhexConfig E;

/*** filetypes ***/

const char* C_HL_extensions[] = { ".c", ".h", ".cpp", nullptr };
const char* C_HL_keywords[] = {
    "switch",    "if",      "while",   "for",    "break",
    "continue",  "return",  "else",    "struct", "union",
    "typedef",   "static",  "enum",    "class",  "case",

    "int|",      "long|",   "double|", "float|", "char|",
    "unsigned|", "signed|", "void|",   nullptr };

struct dhexSyntax HLDB[] = {
    {"c", C_HL_extensions, C_HL_keywords, "//", "/*", "*/",
     HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/*** prototypes ***/

char* dhexPrompt(const char* prompt, void (*callback)(char*, int));

/*** syntax highlighting ***/

int is_separator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != nullptr;
}

void dhexUpdateSyntax(erow* row) {
    row->hl = (unsigned char*)realloc(row->hl, row->rsize);
    memset(row->hl, HL_NORMAL, row->rsize);

    if (E.syntax == nullptr)
        return;

    const char** keywords = E.syntax->keywords;

    const char* scs = E.syntax->singleline_comment_start;
    const char* mcs = E.syntax->multiline_comment_start;
    const char* mce = E.syntax->multiline_comment_end;

    int scs_len = scs ? strlen(scs) : 0;
    int mcs_len = mcs ? strlen(mcs) : 0;
    int mce_len = mce ? strlen(mce) : 0;

    int prev_sep = 1;
    int in_string = 0;
    int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);

    int i = 0;
    while (i < row->rsize) {
        char c = row->render[i];
        unsigned char prev_hl = (i > 0) ? row->hl[i - 1] : (char)HL_NORMAL;

        if (scs_len && !in_string && !in_comment) {
            if (!strncmp(&row->render[i], scs, scs_len)) {
                memset(&row->hl[i], HL_COMMENT, row->rsize - i);
                break;
            }
        }

        if (mcs_len && mce_len && !in_string) {
            if (in_comment) {
                row->hl[i] = HL_MLCOMMENT;
                if (!strncmp(&row->render[i], mce, mce_len)) {
                    memset(&row->hl[i], HL_MLCOMMENT, mce_len);
                    i += mce_len;
                    in_comment = 0;
                    prev_sep = 1;
                    continue;
                }
                else {
                    i++;
                    continue;
                }
            }
            else if (!strncmp(&row->render[i], mcs, mcs_len)) {
                memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
                i += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_STRINGS) {
            if (in_string) {
                row->hl[i] = HL_STRING;
                if (c == '\\' && i + 1 < row->rsize) {
                    row->hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (c == in_string)
                    in_string = 0;
                i++;
                prev_sep = 1;
                continue;
            }
            else {
                if (c == '"' || c == '\'') {
                    in_string = c;
                    row->hl[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if (E.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
            if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
                (c == '.' && prev_hl == HL_NUMBER)) {
                row->hl[i] = HL_NUMBER;
                i++;
                prev_sep = 0;
                continue;
            }
        }

        if (prev_sep) {
            int j;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen - 1] == '|';
                if (kw2)
                    klen--;

                if (!strncmp(&row->render[i], keywords[j], klen) &&
                    is_separator(row->render[i + klen])) {
                    memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != nullptr) {
                prev_sep = 0;
                continue;
            }
        }

        prev_sep = is_separator(c);
        i++;
    }

    int changed = (row->hl_open_comment != in_comment);
    row->hl_open_comment = in_comment;
    if (changed && row->idx + 1 < E.numrows)
        dhexUpdateSyntax(&E.row[row->idx + 1]);
}

fg dhexSyntaxToColor(int hl) {
    switch (hl) {
    case HL_COMMENT:
    case HL_MLCOMMENT:
        return fg::cyan;
    case HL_KEYWORD1:
        return fg::yellow;
    case HL_KEYWORD2:
        return fg::green;
    case HL_STRING:
        return fg::magenta;
    case HL_NUMBER:
        return fg::red;
    case HL_MATCH:
        return fg::blue;
    default:
        return fg::gray;
    }
}

void dhexSelectSyntaxHighlight() {
    E.syntax = nullptr;
    if (E.filename == nullptr)
        return;

    char* ext = strrchr(E.filename, '.');

    for (auto& j : HLDB) {
        struct dhexSyntax* s = &j;
        unsigned int i = 0;
        while (s->filematch[i]) {
            int is_ext = (s->filematch[i][0] == '.');
            if ((is_ext && ext && !strcmp(ext, s->filematch[i])) ||
                (!is_ext && strstr(E.filename, s->filematch[i]))) {
                E.syntax = s;

                int filerow;
                for (filerow = 0; filerow < E.numrows; filerow++) {
                    dhexUpdateSyntax(&E.row[filerow]);
                }

                return;
            }
            i++;
        }
    }
}

/*** row operations ***/

int dhexRowCxToRx(erow* row, int cx) {
    int rx = 0;
    for (int j = 0; j < cx; j++) {
        if (row->chars[j] == '\t')
            rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP);
        rx++;
    }
    return rx;
}

int dhexRowRxToCx(erow* row, int rx) {
    int cur_rx = 0;
    int cx{};
    for (cx = 0; cx < row->size; cx++) {
        if (row->chars[cx] == '\t')
            cur_rx += (KILO_TAB_STOP - 1) - (cur_rx % KILO_TAB_STOP);
        cur_rx++;

        if (cur_rx > rx)
            return cx;
    }
    return cx;
}

void dhexUpdateRow(erow* row) {
    int tabs = 0;
    for (int j = 0; j < row->size; j++)
        if (row->chars[j] == '\t')
            tabs++;

    free(row->render);
    row->render = (char*)malloc(row->size + tabs * (KILO_TAB_STOP - 1) + 1);

    int idx = 0;
    for (int j = 0; j < row->size; j++) {
        if (row->chars[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % KILO_TAB_STOP != 0)
                row->render[idx++] = ' ';
        }
        else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;

    dhexUpdateSyntax(row);
}

void dhexInsertRow(int at, const char* s, size_t len) {
    if (at < 0 || at > E.numrows)
        return;

    E.row = (erow*)realloc(E.row, sizeof(erow) * (E.numrows + 1));
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));
    for (int j = at + 1; j <= E.numrows; j++)
        E.row[j].idx++;

    E.row[at].idx = at;

    E.row[at].size = len;
    E.row[at].chars = (char*)malloc(len + 1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = nullptr;
    E.row[at].hl = nullptr;
    E.row[at].hl_open_comment = 0;
    dhexUpdateRow(&E.row[at]);

    E.numrows++;
    E.dirty++;
}

void dhexFreeRow(erow* row) {
    free(row->render);
    free(row->chars);
    free(row->hl);
}

void dhexDelRow(int at) {
    if (at < 0 || at >= E.numrows)
        return;
    dhexFreeRow(&E.row[at]);
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));
    for (int j = at; j < E.numrows - 1; j++)
        E.row[j].idx--;
    E.numrows--;
    E.dirty++;
}

void dhexRowInsertChar(erow* row, int at, int c) {
    if (at < 0 || at > row->size)
        at = row->size;
    row->chars = (char*)realloc(row->chars, row->size + 2);
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    row->size++;
    row->chars[at] = c;
    dhexUpdateRow(row);
    E.dirty++;
}

void dhexRowAppendString(erow* row, char* s, size_t len) {
    row->chars = (char*)realloc(row->chars, row->size + len + 1);
    memcpy(&row->chars[row->size], s, len);
    row->size += len;
    row->chars[row->size] = '\0';
    dhexUpdateRow(row);
    E.dirty++;
}

void dhexRowDelChar(erow* row, int at) {
    if (at < 0 || at >= row->size)
        return;
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    dhexUpdateRow(row);
    E.dirty++;
}

/*** dhex operations ***/

void dhexMoveCursor(int key) {
    erow* row = (E.cy >= E.numrows) ? nullptr : &E.row[E.cy];

    switch (key) {
    case Key::ARROW_LEFT:
        if (E.cx != 0) {
            E.cx--;
        }
        else if (E.cy > 0) {
            E.cy--;
            E.cx = E.row[E.cy].size;
        }
        break;
    case Key::ARROW_RIGHT:
        if (row && E.cx < row->size) {
            E.cx++;
        }
        else if (row && E.cx == row->size) {
            E.cy++;
            E.cx = 0;
        }
        break;
    case Key::ARROW_UP:
        if (E.cy != 0) {
            E.cy--;
        }
        break;
    case Key::ARROW_DOWN:
        if (E.cy < E.numrows) {
            E.cy++;
        }
        break;
    }

    row = (E.cy >= E.numrows) ? nullptr : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if (E.cx > rowlen) {
        E.cx = rowlen;
    }
}

void dhexInsertChar(int c) {
    // Change to insert if already char

    char chr = c;
    if (chr == 'a') {
        chr = 'A';
    }
    else if (chr == 'b') {
        chr = 'B';
    }
    else if (chr == 'c') {
        chr = 'C';
    }
    else if (chr == 'd') {
        chr = 'D';
    }
    else if (chr == 'e') {
        chr = 'E';
    }
    else if (chr == 'f') {
        chr = 'F';
    }

    if (chr == '0' || chr == '1' || chr == '2' || chr == '3' || chr == '4' || chr == '5' || chr == '6' || chr == '7' || chr == '8' || chr == '9' || chr == 'A' || chr == 'B' || chr == 'C' || chr == 'D' || chr == 'E' || chr == 'F') {

        int c1 = chr;


        if (E.cy == E.numrows) {
            dhexInsertRow(E.numrows, "", 0);
        }

        if (E.row[E.cy].size > E.cx) {
            E.row[E.cy].chars[E.cx] = chr;
            dhexUpdateRow(&E.row[E.cy]);
            dhexMoveCursor(Key::ARROW_RIGHT);

            if ((E.cx + 1) % 3 == 0) {
                dhexMoveCursor(Key::ARROW_RIGHT);
            }
        }
        else {
            dhexRowInsertChar(&E.row[E.cy], E.cx, c1);
            E.cx++;

            if ((E.cx + 1) % 3 == 0) {
                dhexRowInsertChar(&E.row[E.cy], E.cx, ' ');
                E.cx++;
            }
        }

        if (E.cx >= 3 * 16 - 1) {
            E.cx = 0;
            dhexMoveCursor(Key::ARROW_DOWN);
        }
    }
}

void dhexInsertNewline() {
    if (E.cx == 0) {
        dhexInsertRow(E.cy, "", 0);
    }
    else {
        erow* row = &E.row[E.cy];
        dhexInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);
        row = &E.row[E.cy];
        row->size = E.cx;
        row->chars[row->size] = '\0';
        dhexUpdateRow(row);
    }
    E.cy++;
    E.cx = 0;
}

void dhexDelChar() {
    if (E.cy == E.numrows)
        return;
    if (E.cx == 0 && E.cy == 0)
        return;

    erow* row = &E.row[E.cy];
    if (E.cx > 0) {
        dhexRowDelChar(row, E.cx - 1);
        E.cx--;
    }
    else {
        E.cx = E.row[E.cy - 1].size;
        dhexRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
        dhexDelRow(E.cy);
        E.cy--;
    }
}

/*** file i/o ***/

char* dhexRowsToString(int* buflen) {
    int totlen = 0;
    for (int j = 0; j < E.numrows; j++)
        totlen += E.row[j].size + 1;
    *buflen = totlen;

    char* buf = (char*)malloc(totlen);
    char* p = buf;
    for (int j = 0; j < E.numrows; j++) {
        memcpy(p, E.row[j].chars, E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }

    return buf;
}

void dhexSetStatusMessage(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
    va_end(ap);
    E.statusmsg_time = time(nullptr);
}

void dhexClear() {
    E.numrows = 0;
    E.row = nullptr;
}

void dhexOpen(char* filename) {
    // Open Into Hexdata
    hexData = HexTools::HexData(filename);

    free(E.filename);
#ifdef _WIN32
    E.filename = _strdup(filename);
#else
    E.filename = strdup(filename);
#endif
    dhexSelectSyntaxHighlight();

    dhexClear();

    std::string line;
    std::stringstream data;
    data << hexData.Format(1, 0, 16);
    std::getline(data, line);
    while (data.rdstate() == std::ios_base::goodbit) {
        int linelen = line.size();
        while (linelen > 0 &&
            (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;
        dhexInsertRow(E.numrows, line.c_str(), linelen);
        std::getline(data, line);
    }
    E.dirty = 0;

    E.cx = 0;
    E.cy = 0;
}

void dhexSave() {
    std::string data;
    for (int i = 0; i < E.numrows; i++) {
        erow row = E.row[i];
        data += row.chars;
    }

    std::vector<HexTools::HexByte> bytes;
    HexTools::HexByte byte;
    for (int i = 0; i < data.size(); i++) {
        if ((i + 1) % 3 == 0) {
            bytes.push_back(byte);
        }
        else {
            if ((i + 2) % 3 == 0) {
                byte.b = data[i];
            }
            else {
                byte.a = data[i];
            }
        }
    }

    hexData.data = bytes;

    if (E.filename == nullptr) {
        E.filename = dhexPrompt("Save as: %s (ESC to cancel)", nullptr);
        if (E.filename == nullptr) {
            dhexSetStatusMessage("Save aborted");
            return;
        }
        dhexSelectSyntaxHighlight();
    }

    hexData.Save(E.filename);

    dhexSetStatusMessage("%d bytes written to disk", hexData.data.size());
}

void dhexLoad() {
    std::string filename = dhexPrompt("Open file: %s (ESC to cancel)", nullptr);
    if (filename.size() == 0) {
        dhexSetStatusMessage("Open aborted");
        return;
    }

    dhexOpen((char*)filename.c_str());
}

/*** find ***/

void dhexFindCallback(char* query, int key) {
    static int last_match = -1;
    static int direction = 1;

    static int saved_hl_line;
    static char* saved_hl = nullptr;

    if (saved_hl) {
        memcpy(E.row[saved_hl_line].hl, saved_hl, E.row[saved_hl_line].rsize);
        free(saved_hl);
        saved_hl = nullptr;
    }

    if (key == Key::ENTER || key == Key::ESC) {
        last_match = -1;
        direction = 1;
        return;
    }
    else if (key == Key::ARROW_RIGHT || key == Key::ARROW_DOWN) {
        direction = 1;
    }
    else if (key == Key::ARROW_LEFT || key == Key::ARROW_UP) {
        direction = -1;
    }
    else {
        last_match = -1;
        direction = 1;
    }

    if (last_match == -1)
        direction = 1;
    int current = last_match;
    for (int i = 0; i < E.numrows; i++) {
        current += direction;
        if (current == -1)
            current = E.numrows - 1;
        else if (current == E.numrows)
            current = 0;

        erow* row = &E.row[current];
        char* match = strstr(row->render, query);
        if (match) {
            last_match = current;
            E.cy = current;
            E.cx = dhexRowRxToCx(row, match - row->render);
            E.rowoff = E.numrows;

            saved_hl_line = current;
            saved_hl = (char*)malloc(row->rsize);
            memcpy(saved_hl, row->hl, row->rsize);
            memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }
}

void dhexFind() {
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    char* query =
        dhexPrompt("Search: %s (Use ESC/Arrows/Enter)", dhexFindCallback);

    if (query) {
        free(query);
    }
    else {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
    }
}

/*** output ***/

void dhexScroll() {
    E.rx = 0;
    if (E.cy < E.numrows) {
        E.rx = dhexRowCxToRx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.rowoff) {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows - 1) {
        E.rowoff = E.cy - E.screenrows + 1;
    }
    if (E.rx < E.coloff) {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) {
        E.coloff = E.rx - E.screencols + 1;
    }
}

void dhexDrawRows(std::string& ab) {
    ab.append(" Offset   | 0123456789ABCDEF | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    ab.append(erase_to_eol());
    ab.append("\r\n");
    for (int y = 0; y < E.screenrows; y++) {
        int filerow = y + E.rowoff;

        std::stringstream stream;
        stream << std::hex << (y + E.rowoff) * 16;

        std::string str = stream.str();

        int left = 8 - str.size();

        for (int i = 0; i < left; i++) str = "0" + str;

        str += "h | ";

        if (filerow >= E.numrows) {
            for (int i = 0; i < 16; i++) str += ".";
        }
        else {
            // Construct String

            std::string chars;
            std::string data = E.row[filerow].chars;

            HexTools::HexData hd;

            std::vector<HexTools::HexByte> bytes;
            HexTools::HexByte byte;
            for (int i = 0; i < data.size(); i++) {
                if ((i + 1) % 3 == 0) {
                    bytes.push_back(byte);
                }
                else {
                    if ((i + 2) % 3 == 0) {
                        byte.b = data[i];
                    }
                    else {
                        byte.a = data[i];
                    }
                }
            }

            hd.data = bytes;

            std::string hdStr = hd.ToString();

            for (int i = 0; i < hdStr.size(); i++) {
                if (hdStr[i] < 32 || hdStr[i] > 126) {
                    hdStr[i] = '.';
                }
            }

            str += hdStr;

            if (str.size() != 28) {
                while (str.size() != 28) str += '.';
            }
        }

        str += " | ";

        ab.append(str);
        if (filerow >= E.numrows) {
            if (E.numrows == 0 && y == E.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Devparty Text Editor");
                if (welcomelen > E.screencols) welcomelen = E.screencols;
                int padding = (E.screencols - welcomelen) / 2;
                if (padding) {
                    ab.append("~");
                    padding--;
                }
                while (padding--)
                    ab.append(" ");
                ab.append(welcome);
            }
            else {
                ab.append("~");
            }
        }
        else {
            int len = E.row[filerow].rsize - E.coloff;
            if (len < 0)
                len = 0;
            if (len > E.screencols)
                len = E.screencols;
            char* c = &E.row[filerow].render[E.coloff];
            unsigned char* hl = &E.row[filerow].hl[E.coloff];
            fg current_color =
                fg::black;  // black is not used in dhexSyntaxToColor
            int j;
            for (j = 0; j < len; j++) {
                if (iscntrl(c[j])) {
                    char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                    ab.append(color(style::reversed));
                    ab.append(std::string(&sym, 1));
                    ab.append(color(style::reset));
                    if (current_color != fg::black) {
                        ab.append(color(current_color));
                    }
                }
                else if (hl[j] == HL_NORMAL) {
                    if (current_color != fg::black) {
                        ab.append(color(fg::reset));
                        current_color = fg::black;
                    }
                    ab.append(std::string(&c[j], 1));
                }
                else {
                    fg color = dhexSyntaxToColor(hl[j]);
                    if (color != current_color) {
                        current_color = color;
                        ab.append(Term::color(color));
                    }
                    ab.append(std::string(&c[j], 1));
                }
            }
            ab.append(color(fg::reset));
        }

        ab.append(erase_to_eol());
        ab.append("\r\n");
    }
}

void dhexDrawStatusBar(std::string& ab) {
    ab.append(color(style::reversed));
    char status[80], rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
        E.filename ? E.filename : "[No Name]", E.numrows,
        E.dirty ? "(modified)" : "");
    int rlen =
        snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
            E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);
    if (len > E.screencols)
        len = E.screencols;
    ab.append(std::string(status, len));
    while (len < E.screencols) {
        if (E.screencols - len == rlen) {
            ab.append(std::string(rstatus, rlen));
            break;
        }
        else {
            ab.append(" ");
            len++;
        }
    }
    ab.append(color(style::reset));
    ab.append("\r\n");
}

void dhexDrawMessageBar(std::string& ab) {
    ab.append(erase_to_eol());
    int msglen = strlen(E.statusmsg);
    if (msglen > E.screencols)
        msglen = E.screencols;
    if (msglen && time(nullptr) - E.statusmsg_time < 5)
        ab.append(std::string(E.statusmsg, msglen));
}

void dhexRefreshScreen() {
    dhexScroll();

    std::string ab;
    ab.reserve(16 * 1024);

    ab.append(cursor_off());
    ab.append(move_cursor(1, 1));

    dhexDrawRows(ab);
    dhexDrawStatusBar(ab);
    dhexDrawMessageBar(ab);

    ab.append(move_cursor((E.cy - E.rowoff) + 2, (E.rx - E.coloff) + 13 + 19));

    ab.append(cursor_on());

    Term::write(ab);
}

/*** input ***/

char* dhexPrompt(const char* prompt, void (*callback)(char*, int)) {
    size_t bufsize = 128;
    char* buf = (char*)malloc(bufsize);

    size_t buflen = 0;
    buf[0] = '\0';

    while (true) {
        dhexSetStatusMessage(prompt, buf);
        dhexRefreshScreen();

        int c = Term::read_key();
        if (c == Key::DEL || c == Key::CTRL + 'h' || c == Key::BACKSPACE) {
            if (buflen != 0)
                buf[--buflen] = '\0';
        }
        else if (c == Key::ESC) {
            dhexSetStatusMessage("");
            if (callback)
                callback(buf, c);
            free(buf);
            return nullptr;
        }
        else if (c == Key::ENTER) {
            if (buflen != 0) {
                dhexSetStatusMessage("");
                if (callback)
                    callback(buf, c);
                return buf;
            }
        }
        else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = (char*)realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }

        if (callback)
            callback(buf, c);
    }
}

void dhexGoto() {
    std::string offset = dhexPrompt("Goto Offset: %s (Use ESC/Arrows/Enter)", nullptr);

    unsigned int y;
    std::stringstream ss;
    ss << std::hex << offset;
    ss >> y;

    y = floor(y / 16);

    E.cy = y;

    unsigned int x;
    std::stringstream ss1;
    ss1 << std::hex << offset[offset.length() - 1];
    ss1 >> x;

    E.cx = x * 3;
}

bool dhexProcessKeypress() {
    static int quit_times = KILO_QUIT_TIMES;

    int c = Term::read_key();

    switch (c) {
    case Key::ENTER:
        if(E.row[E.cy].size == 16*3) dhexMoveCursor(Key::ARROW_DOWN);
        break;

    case Key::CTRL + 'q':
        if (E.dirty && quit_times > 0) {
            dhexSetStatusMessage(
                "WARNING!!! File has unsaved changes. "
                "Press Ctrl-Q %d more times to quit.",
                quit_times);
            quit_times--;
            return true;
        }
        return false;
        break;

    case Key::CTRL + 's':
        dhexSave();
        break;

    case Key::CTRL + 'o':
        dhexLoad();
        break;

    case Key::HOME:
        E.cx = 0;
        break;

    case Key::END:
        if (E.cy < E.numrows)
            E.cx = E.row[E.cy].size;
        break;

    case Key::CTRL + 'g':
        dhexGoto();
        break;

    case Key::BACKSPACE:
    case Key::DEL:
        if (c == Key::DEL) {
            dhexMoveCursor(Key::ARROW_RIGHT);
        }
        else if (c == Key::BACKSPACE) {
            // do a LONG list of things
            if (E.row[E.cy].size > E.cx) {
                E.row[E.cy].chars[E.cx] = '0';
                dhexUpdateRow(&E.row[E.cy]);
            }
            dhexMoveCursor(Key::ARROW_LEFT);
            if ((E.cx + 1) % 3 == 0) {
                dhexMoveCursor(Key::ARROW_LEFT);
            }
        }
        break;

    case Key::ARROW_UP:
    case Key::ARROW_DOWN:
    case Key::ARROW_LEFT:
    case Key::ARROW_RIGHT:
        if ((c == Key::ARROW_DOWN && E.row[E.cy].size == 16 * 3)|| (c == Key::ARROW_RIGHT && (E.row[E.cy].size == 16*3 || E.cx != E.row[E.cy].size)) || (c != Key::ARROW_DOWN && c != Key::ARROW_RIGHT)) dhexMoveCursor(c);
        if ((E.cx + 1) % 3 == 0) {
            if (c == Key::ARROW_RIGHT) {
                dhexMoveCursor(c);
            }
            else if (c == Key::ARROW_LEFT) {
                dhexMoveCursor(c);
            }
        }
        if (E.cx >= 3 * 16 - 1 && c == Key::ARROW_RIGHT) {
            E.cx = 0;
            dhexMoveCursor(Key::ARROW_DOWN);
        }
        if (E.cx >= 3 * 16 - 1 && c == Key::ARROW_LEFT) {
            dhexMoveCursor(Key::ARROW_LEFT);
            dhexMoveCursor(Key::ARROW_LEFT);
        }
        break;

    case Key::CTRL + 'l':
    case Key::ESC:
        break;

    case Key::TAB:
        dhexInsertChar('\t');
        break;

    default:
        dhexInsertChar(c);
        break;
    }

    quit_times = KILO_QUIT_TIMES;
    return true;
}

/*** init ***/

void initdhex() {
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = nullptr;
    E.dirty = 0;
    E.filename = nullptr;
    E.statusmsg[0] = '\0';
    E.statusmsg_time = 0;
    E.syntax = nullptr;

    Term::get_term_size(E.screenrows, E.screencols);
    E.screenrows -= 3;
}

int main(int argc, char* argv[]) {
    // We must put all code in try/catch block, otherwise destructors are not
    // being called when exception happens and the terminal is not put into
    // correct state.
    try {
        // check if the terminal is capable of handling input
        if (!Term::is_stdin_a_tty()) {
            std::cout << "The terminal is not attached to a TTY and therefore can't catch user input. Exiting...\n";
            return 1;
        }
        Terminal term(true, true, false, false);
        initdhex();
        if (argc >= 2) {
            dhexOpen(argv[1]);
        }

        dhexSetStatusMessage(
            "HELP: Ctrl-S = save | Ctrl-O = open | Ctrl-G = goto | Ctrl-Q = quit");

        dhexRefreshScreen();
        while (dhexProcessKeypress()) {
            dhexRefreshScreen();
        }
    }
    catch (const std::runtime_error& re) {
        std::cerr << "Runtime error: " << re.what() << std::endl;
        return 2;
    }
    catch (...) {
        std::cerr << "Unknown error." << std::endl;
        return 1;
    }
    return 0;
}