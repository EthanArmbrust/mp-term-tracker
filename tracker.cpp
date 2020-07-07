#include <iostream>
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <locale.h>
#include <sstream>
#include "MemoryWatch/MemWatchEntry.h"
#include <wchar.h>

#include "DolphinProcess/DolphinAccessor.h"
#include "Common/CommonUtils.h"

using namespace std;

#define MEM_START 0x80000000
#define INVENTORY_START 0x804578CC

#define REFRESH_RATE 30

struct prime_item
{
    string name;
    u32 offset;
    bool isCounter;
    int counter;
    MemWatchEntry *entry;
};

struct prime_item_list
{
    string label;
    vector<prime_item> item_list;
    int width = 0;
    bool containsCounter = false;
};

void init_curses()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_RED);
    init_pair(2, COLOR_BLACK, COLOR_GREEN);
    //init_pair(1, COLOR_WHITE, COLOR_BLACK);
    //init_pair(2, COLOR_BLACK, COLOR_WHITE);
}
void draw_rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2 - x1);
    mvhline(y2, x1, 0, x2 - x1);
    mvvline(y1, x1, 0, y2 - y1);
    mvvline(y1, x2, 0, y2 - y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}
int find_width_pil(vector<prime_item> &l)
{
    int width = 0;
    for (auto &i : l)
    {
        if (width < i.name.length())
        {
            width = i.name.length();
        }
    }
    return width;
}
int count_artifacts(vector<prime_item> &l)
{
    int sum = 0;
    for (prime_item &art : l)
    {
        if (art.entry->readMemoryFromRAM() == Common::MemOperationReturnCode::justK)
        {
            if (stoi(art.entry->getStringFromMemory()))
            {
                sum++;
            }
        }
    }
    return sum;
}

prime_item create_item(string name, u32 offset, bool isCounter)
{
    MemWatchEntry *item_entry = new MemWatchEntry;
    item_entry->setTypeAndLength(Common::MemType::type_word, sizeof(u16));
    item_entry->setConsoleAddress(INVENTORY_START);
    item_entry->setBoundToPointer(true);
    item_entry->addOffset(offset);

    prime_item item = {name, offset, isCounter, 0, item_entry};
    return item;
}

vector<prime_item> init_artifact_list()
{
    prime_item nature = create_item("Artifact of Nature", 0x3EB, false);
    prime_item sun = create_item("Artifact of Sun", 0x3F0, false);
    prime_item spirit = create_item("Artifact of Spirit", 0x400, false);
    prime_item elder = create_item("Artifact of Elder", 0x3C0, false);
    prime_item strength = create_item("Artifact of Strength", 0x3B8, false);
    prime_item lifegiver = create_item("Artifact of Lifegiver", 0x3D0, false);
    prime_item world = create_item("Artifact of World", 0x3F8, false);
    prime_item wild = create_item("Artifact of Wild", 0x3C8, false);
    prime_item chozo = create_item("Artifact of Chozo", 0x3E0, false);
    prime_item truth = create_item("Artifact of Truth", 0x3B0, false);
    prime_item newborn = create_item("Artifact of Newborn", 0x408, false);
    prime_item warrior = create_item("Artifact of Warrior", 0x3D8, false);
    return {nature, sun, spirit, elder, strength, lifegiver, world, wild, chozo, truth, newborn, warrior};
}

void other_art_thing(vector<prime_item> &l)
{
    for (auto &a : l)
    {
        int offset = a.entry->getPointerOffset(0);
        a.entry->setPointerOffset(offset + 4, 0);
    }
}

prime_item_list init_beams()
{
    prime_item wave_beam = create_item("Wave Beam", 0x2DC, false);
    prime_item ice_beam = create_item("Ice Beam", 0x2D4, false);
    prime_item plasma_beam = create_item("Plasma Beam", 0x2E4, false);
    vector<prime_item> l = {wave_beam, ice_beam, plasma_beam};
    int width = find_width_pil(l);
    return {"Beams", l, width, false};
}
prime_item_list init_suits()
{
    prime_item varia = create_item("Varia Suit", 0x37C, false);
    prime_item gravity = create_item("Gravity Suit", 0x374, false);
    prime_item phazon = create_item("Phazon Suit", 0x384, false);
    vector<prime_item> l = {varia, gravity, phazon};
    int width = find_width_pil(l);
    return {"Suits", l, width, false};
}
prime_item_list init_combos()
{
    prime_item supers = create_item("Super Missiles", 0x324, false);
    prime_item wavebuster = create_item("Wavebuster", 0x3AC, false);
    prime_item ice_spreader = create_item("Ice Spreader", 0x33C, false);
    prime_item flamethrower = create_item("Flamethrower", 0x30C, false);
    vector<prime_item> l = {supers, wavebuster, ice_spreader, flamethrower};
    int width = find_width_pil(l);
    return {"Beam Combos", l, width, false};
}
prime_item_list init_visors()
{
    prime_item scan = create_item("Scan Visor", 0x2F4, false);
    prime_item thermal = create_item("Thermal Visor", 0x314, false);
    prime_item xray = create_item("X-Ray Visor", 0x334, false);
    vector<prime_item> l = {scan, thermal, xray};
    int width = find_width_pil(l);
    return {"Visors", l, width, false};
}
prime_item_list init_morph_upgrades()
{
    prime_item morph = create_item("Morph Ball", 0x34C, false);
    prime_item bombs = create_item("Morph Ball Bombs", 0x2FC, false);
    prime_item boost = create_item("Boost Ball", 0x35C, false);
    prime_item spider = create_item("Spider Ball", 0x364, false);
    prime_item pb = create_item("Power Bombs", 0x304, true);
    vector<prime_item> l = {morph, bombs, boost, spider, pb};
    int width = find_width_pil(l);
    return {"Morph Upgrades", l, width, true};
}

prime_item_list init_misc()
{
    prime_item missile = create_item("Missiles", 0x2EC, true);
    prime_item etank = create_item("Energy Tanks", 0x38C, true);
    prime_item sj = create_item("Space Jump Boots", 0x340, false);
    prime_item charge = create_item("Charge Beam", 0x318, false);
    prime_item grapple = create_item("Grapple Beam", 0x32C, false);
    vector<prime_item> l = {missile, etank, sj, charge, grapple};
    int width = find_width_pil(l);
    return {"Miscellaneous", l, width, true};
}

string format_IGT(string raw_IGT)
{
    double dIGT;
    try
    {
        dIGT = stod(raw_IGT);
    }
    catch (const std::invalid_argument &ia)
    {
        return "??:??:??.???";
    }
    int IGT = (int)dIGT;
    int hours = IGT / 3600;
    int minutes = (IGT - (hours * 3600)) / 60;
    int seconds = IGT - ((hours * 3600) + (minutes * 60));
    int milliseconds = (int)(dIGT * 1000) % 1000;
    char buffer[50];
    sprintf(buffer, "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
    return buffer;
}

void draw_list(prime_item_list &l, int y, int x)
{
    int height = l.item_list.size() + 1;
    int width = l.width;
    bool containsCounter = l.containsCounter;
    for (int i = 0; i < l.item_list.size(); i++)
    {
        string item_output = l.item_list[i].name;
        if (l.item_list[i].entry->readMemoryFromRAM() == Common::MemOperationReturnCode::justK)
        {
            if (stoi(l.item_list[i].entry->getStringFromMemory()))
            {
                attron(COLOR_PAIR(2));
                mvaddstr(y + i + 1, 2 + x, item_output.c_str());
                attroff(COLOR_PAIR(2));
                if (l.item_list[i].isCounter)
                {
                    string value = l.item_list[i].entry->getStringFromMemory();
                    int val = stoi(value);
                    char val_buffer[4];
                    sprintf(val_buffer, "%03d", val);
                    mvaddstr(y + i + 1, x + 2 + l.width + 2, val_buffer);
                }
            }
            else
            {
                attron(COLOR_PAIR(1));
                mvaddstr(y + i + 1, 2 + x, item_output.c_str());
                attroff(COLOR_PAIR(1));
                if (l.item_list[i].isCounter)
                {
                    char val_buffer[4] = "   ";
                    mvaddstr(y + i + 1, x + 2 + l.width + 2, val_buffer);
                }
            }
        }
    }
    if (containsCounter)
    {
        width += 5;
    }
    draw_rectangle(y, x, y + height, x + width + 2);
    mvaddstr(y, x, l.label.c_str());
}

wstring string_to_checkbox(string input)
{
    if (input == "0")
    {
        return L"\u2610";
    }
    else
    {
        return L"\u2611";
    }
}

vector<string> split_message(string message, int width)
{
    stringstream ss(message);
    string s;
    vector<string> word_array;
    while (ss >> s)
    {
        word_array.push_back(s);
    }
    vector<string> line_array;
    string current_line = "";
    while (!word_array.empty())
    {
        if (!(current_line.length() + word_array[0].length() + 1 <= width))
        {
            line_array.push_back(current_line);
            current_line = "";
        }
        current_line += word_array[0] + " ";
        word_array.erase(word_array.begin());
    }
    if (current_line != "")
    {
        line_array.push_back(current_line);
    }
    return line_array;
}

bool could_not_hook(string message)
{
    //Return true if user has requested retry
    //Return false if user is quitting
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);

    int new_x = max_x - 4;
    int new_y = max_y - 4;

    WINDOW *win = newwin(new_y, new_x, 2, 2);

    vector<string> message_array = split_message(message, new_x - 2);
    int start_row = (new_y / 2) - (message_array.size() / 2);
    wclear(win);

    for (int i = 0; i < message_array.size(); i++)
    {
        int start_col = (new_x / 2) - (message_array[i].length() / 2);
        mvwaddstr(win, start_row + i, start_col, message_array[i].c_str());
    }
    string controls = "[R] - Retry : [Q] - Quit";
    mvwaddstr(win, new_y - 2, (new_x / 2) - (controls.length() / 2), controls.c_str());
    box(win, 0, 0);
    wrefresh(win);

    for (;;)
    {
        char input = wgetch(win);
        if (input == 'q' || input == 'Q')
        {
            return false;
        }
        if (input == 'r' || input == 'R')
        {
            return true;
        }
        wrefresh(win);
    }

    return false;
}

int main()
{
    init_curses();
    DolphinComm::DolphinAccessor::init();
    DolphinComm::DolphinAccessor::hook();
    DolphinComm::DolphinAccessor::updateRAMCache();
    MemWatchEntry igt_entry;
    igt_entry.setTypeAndLength(Common::MemType::type_double, sizeof(double));
    igt_entry.setConsoleAddress(0x804578CC);
    igt_entry.setBoundToPointer(true);
    igt_entry.addOffset(0xA0);
    prime_item_list beams = init_beams();
    prime_item_list morph_upgrades = init_morph_upgrades();
    prime_item_list visors = init_visors();
    prime_item_list suits = init_suits();
    prime_item_list combos = init_combos();
    prime_item_list misc = init_misc();
    vector<prime_item> artifacts = init_artifact_list();
    //other_art_thing(artifacts);
    int width = find_width_pil(artifacts);
    prime_item_list artifact_list = {"Artifacts", artifacts, width, false};

    for (;;)
    {
        erase();
        DolphinComm::DolphinAccessor::hook();
        while (DolphinComm::DolphinAccessor::getStatus() != DolphinComm::DolphinAccessor::DolphinStatus::hooked)
        {
            if (!could_not_hook("Could not hook into Dolphin"))
            {
                endwin();
                return 0;
            }
            else
            {
                DolphinComm::DolphinAccessor::hook();
            }
        }
        DolphinComm::DolphinAccessor::updateRAMCache();
        string game_id = DolphinComm::DolphinAccessor::getFormattedValueFromCache(Common::dolphinAddrToOffset(MEM_START), Common::MemType::type_string, sizeof(char) * 6, Common::MemBase::base_none, true);
        string IGT_raw;
        string wave_val;
        if (igt_entry.readMemoryFromRAM() == Common::MemOperationReturnCode::justK)
        {
            IGT_raw = igt_entry.getStringFromMemory();
        }
        mvprintw(0, 0, ("IGT: " + format_IGT(IGT_raw)).c_str());
        draw_list(beams, 1, 0);
        draw_list(morph_upgrades, 1, 20);
        draw_list(visors, 6, 0);
        draw_list(misc, 8, 20);
        draw_list(combos, 11, 0);
        draw_list(suits, 17, 0);
        draw_list(artifact_list, 15, 20);

        //int art_count = count_artifacts(artifacts);
        //mvaddstr(14, 25, ("Artifacts " + to_string(art_count) + " ").c_str());

        wrefresh(stdscr);
        this_thread::sleep_for(chrono::milliseconds(1000 / REFRESH_RATE));
    }
    endwin();
    return 0;
}
