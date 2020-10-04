#include <curses.h>
#include <string>

class Board {
private:
    std::string board;
    int width;
    int height;
    int mines;

    int flaggedMines;
    int incorrectFlags;
    int uncoverdArea;

    int arrowCount;
    int curX;
    int curY;
    int curBlink;

    bool isDead;

public:
    Board(int height, int width, int mines) {
        this->board = std::string(height * width, '.');
        this->height = height;
        this->width = width;
        this->mines = mines;
        this->uncoverdArea = -1;// sentinal
        this->curX = width / 2;
        this->curY = height / 2;
        this->curBlink = 0;
        this->arrowCount = 0;
        this->isDead = false;
    }

    bool IsDead() {
        return isDead;
    }

    bool IsWin() {
        return uncoverdArea >= (width * height) - mines;
    }

    // Process events for char; returns true if game over
    void OnChar(char c)
    {
        if (c == '\033' && arrowCount == 0){
            arrowCount = 1;
            return;
        } else if (c == '[' && arrowCount == 1){
            arrowCount = 2;
            return;
        } else if (arrowCount == 2){
            switch (c)
            {
                case 'A':
                    c = 'w';
                    break;
                case 'B':
                    c = 's';
                    break;
                case 'C':
                    c = 'd';
                    break;
                case 'D':
                    c = 'a';
                    break;
            }
        }
        arrowCount = 0;
        switch (std::tolower(c))
        {
            case 'w':
                curY = (height + curY - 1) % height;
                break;
            case 'a':
                curX = (width + curX - 1) % width;
                break;
            case 's':
                curY = (curY + 1) % height;
                break;
            case 'd':
                curX = (curX + 1) % width;
                break;
            case 'f':
            case '\'':
                if (!IsUncoverd(curX, curY)) {
                    board[curY * width + curX] = \
                        board[curY * width + curX] == '*' ? 'X' : \
                        board[curY * width + curX] == '>' ? '.' : \
                        board[curY * width + curX] == 'X' ? '*' : '>';
                }
                break;
            case '\r':
                if (!IsFlag(curX, curY)) {
                    UpdateForHit(curX, curY);
                }
                return;
            }
        return;
    }

    void Draw(bool drawHiddenMines) {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++) {
                char toDraw = drawHiddenMines ? board[y * width + x] : HideMines(board[y * width + x]);
                int color = GetColor(toDraw);

                if (color > 0) {
                    attron(COLOR_PAIR(color));
                    mvaddch(y + 1, x + 1, toDraw);
                    attroff(COLOR_PAIR(color));
                }
                else {
                    mvaddch(y + 1, x + 1, toDraw);
                }
            }
        }

        move(curY+1, curX+1);
        refresh();
    }
private:

    int GetColor(char c) {
        switch (c)
        {
            case '1': return 1;
            case '2': return 1;
            case '3': return 2;
            case '4': return 2;
            case '5': return 3;
            case '6': return 3;
            case '7': return 4;
            case '8': return 4;
            case '>': return 5;
            case 'X': return 5;
            case '.': return 6;
            default: return -1;
        }
    }

    void UpdateForHit(int x, int y) {
        if (uncoverdArea < 0) {
            CreateMines();
        }

        if (!IsInBounds(x, y) || IsUncoverd(x, y)) {
            return;
        }

        if (HasMine(x, y)) {
            isDead = true;
            return;
        }

        uncoverdArea++;
        int near = HasMine(x + 1, y) +
                   HasMine(x - 1, y) +
                   HasMine(x, y + 1) +
                   HasMine(x, y - 1) +
                   HasMine(x + 1, y + 1) +
                   HasMine(x - 1, y + 1) +
                   HasMine(x + 1, y - 1) +
                   HasMine(x - 1, y - 1);

        if (near > 0) {
            board[y * width + x] = 48 + near;
        }
        else {
            board[y * width + x] = ' ';
            UpdateForHit(x + 1, y);
            UpdateForHit(x - 1, y);
            UpdateForHit(x, y + 1);
            UpdateForHit(x, y - 1);
            UpdateForHit(x + 1, y + 1);
            UpdateForHit(x - 1, y + 1);
            UpdateForHit(x + 1, y - 1);
            UpdateForHit(x - 1, y - 1);
        }
    }

    bool HasMine(int x, int y) {
        return IsInBounds(x, y) &&
               (board[y * width + x] == '*' || board[y * width + x] == 'X');
    }

    bool IsInBounds(int x, int y) {
        return x >= 0 && x < width &&
               y >= 0 && y < height;
    }

    bool IsFlag(int x, int y) {
        return board[y * width + x] == '>' ||
               board[y * width + x] == 'X';
    }

    bool IsUncoverd(int x, int y) {
        return board[y * width + x] == ' ' ||
               (board[y * width + x] >= '1' && board[y * width + x] <= '8');
    }

    void CreateMines() {
        uncoverdArea = 0;
        int minesCreated = 0;
        while (minesCreated < mines) {
            int x = rand() % width;
            int y = rand() % height;
            if ((curX != x || curY != y) && !HasMine(x, y)) {
                board[y * width + x] = board[y * width + x] == '>' ? 'X' : '*';
                minesCreated++;
            }
        }
    }

    char HideMines(char c){
        switch (c)
        {
        case '*': return '.';
        case 'X': return '>';
        default: return c;
        }
    }
};

int main() {
    srand(time(NULL));
    initscr();
    cbreak();

    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_YELLOW);
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);

    while (1)
    {
        clear();
        curs_set(1);
        echo();
        refresh();
        mvaddwstr(0, 0, L"Board Size (H, W): ");

        char cSize[20];
        getnstr(cSize, 20);
        std::string sSize(cSize);

        auto split = sSize.find(',');
        split = split == std::string::npos ? sSize.find(' ') :
                split;

                int w, h;
                if (split == std::string::npos)
                {
                    w = h = std::stoi(sSize);
        }
        else {
            h = std::stoi(sSize.substr(0, split));
            w = std::stoi(sSize.substr(split+1));
        }

        if (h < 5 || w < 5 || h > LINES - 2 || w > COLS - 2){
            noecho();
            curs_set(0);
            clear();
            wchar_t buff[255];
            swprintf(buff, 255, L"Size too big/small (Max H/W): %i / %i", LINES - 2, COLS - 2);
            mvaddwstr(0, 0, buff);
            getch();
            continue;
        }

        clear();
        mvaddwstr(0, 0, L"Mines: ");

        char cMines[20];
        getnstr(cMines, 20);
        int nMines = std::stoi(std::string(cMines));

        Board board(h, w, nMines);
        clear();
        noecho();
        refresh();
        while (!board.IsDead() && !board.IsWin())
        {
            board.Draw(false);
            char action = getchar();
            if (action != -1) {
                board.OnChar(action);
            }
        }

        curs_set(0);
        refresh();
        board.Draw(true);

        if (board.IsDead()){
            mvaddwstr(LINES / 2, COLS / 2 - 5, L"GAME OVER!");
            mvaddwstr(LINES / 2+1, COLS / 2, L"Try Again?");
        }
        else {
            mvaddwstr(LINES / 2, COLS / 2 - 4, L"You Won!");
            mvaddwstr(LINES / 2+1, COLS / 2, L"Play Again?");
        }
        char c = getch();
        if (c != 'y' && c != 'Y') {
            break;
        }
    }

    endwin();
    return 0;
}