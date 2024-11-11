#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <mach-o/dyld.h>

using namespace std;

std::string getExecutablePath() {
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        std::cerr << "Buffer too small; need size " << size << std::endl;
        return "";
    }
    return std::filesystem::path(path).parent_path().string();
}


std::string getFontPath(const std::string& fontFileName) {
    std::string execPath = getExecutablePath();
    return execPath + "/resources/" + fontFileName;
}


bool isDraggingGlobal = false;
bool isVertical = true;

class Tile{
public:
    bool hasShip;
    bool wasShot;
    Tile* next;
    Tile* prev;
    Tile(float x, float y, float size, int i, int j) : hasShip(false), wasShot(false) {
        rectangle.setSize(sf::Vector2f(size, size));
        rectangle.setPosition(x, y);
        rectangle.setFillColor(sf::Color::Blue);
        rectangle.setOutlineThickness(2);
        rectangle.setOutlineColor(sf::Color::White);
        row = i; col = j;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }
    bool state(){
        return hasShip;
    }
    sf::FloatRect getBound(){
        return rectangle.getGlobalBounds();
    }
    void changeColor(sf::Color color){
        rectangle.setFillColor(color);
    }
    sf::Vector2f getPos(){
        return rectangle.getPosition();
    }
    bool kill(Tile* head){
        Tile* curr = head->next;
        bool b = true;
        while(curr != head){
            if(!curr->hasShip || !curr->wasShot){
                b = false;
                break;
            }
            curr = curr->next;
        }
        return b;
    }
    bool getShot(sf::RenderWindow& window, sf::Event event, int &hp, vector<vector<Tile> > &grid, sf::Sound &sound, sf::Sound &sound2){
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect bounds = rectangle.getGlobalBounds();
        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if(event.type == sf::Event::MouseButtonPressed && !wasShot){
                if(hasShip){
                    rectangle.setFillColor(sf::Color::White);
                    wasShot = true;
                    sound.play();
                    hp--;
                    if(kill(this)){
                        vector<vector<int> > dirs = {{0,1},{0,-1},{1,0},{-1,0},{1,1},{1,-1},{-1,-1},{-1,1}};
                        Tile* curr = this;
                        for(int i = 0; i < 8; i++){
                            int newRow = curr->row + dirs[i][0];
                            int newCol = curr->col + dirs[i][1];
                            if(newRow >= 0 && newRow < 10 && newCol >= 0 && newCol < 10 && !grid[newRow][newCol].hasShip){
                                grid[newRow][newCol].changeColor(sf::Color::Green);
                                grid[newRow][newCol].wasShot = true;
                            }
                        }
                        curr = curr->next;
                        while(curr != this){
                            for(int i = 0; i < 8; i++){
                                int newRow = curr->row + dirs[i][0];
                                int newCol = curr->col + dirs[i][1];
                                if(newRow >= 0 && newRow < 10 && newCol >= 0 && newCol < 10 && !grid[newRow][newCol].hasShip){
                                    grid[newRow][newCol].changeColor(sf::Color::Green);
                                }
                            }
                            curr = curr->next;
                        }
                    }
                } else{
                    rectangle.setFillColor(sf::Color::Green);
                    wasShot = true;
                    sound2.play();
                    return 0;
                }
                
            }
        }
        return 1;
    }


private:
    int row,col;
    sf::RectangleShape rectangle;
};

bool canPlace(int r,int c, int len, bool vertical, vector<vector<Tile> > &grid){
    vector<vector<int> > dirs = {{0,1},{0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,-1}, {-1,1}};
    if(vertical && r + len > 10) return false;
    if(!vertical && c + len > 10) return false;
    for(int i = 0; i < len; i++){
        if(vertical && grid[r + i][c].state()) return false;
        else if(!vertical && grid[r][c + i].state()) return false;
    }
    if(vertical){
        for(int i = 0; i < len; i++){
            for(int k = 0; k < 8; k++){
                int newr = r + i + dirs[k][0], newc = c + dirs[k][1];
                if(newr >= 0 && newr < 10 && newc >= 0 && newc < 10){
                    if(grid[newr][newc].state()) return false;
                }
            }
        }
    } else{
        for(int i = 0; i < len; i++){
            for(int k = 0; k < 8; k++){
                int newr = r + dirs[k][0], newc = c + i + dirs[k][1];
                if(newr >= 0 && newr < 10 && newc >= 0 && newc < 10){
                    if(grid[newr][newc].state()) return false;
                }
            }
        }
    }
    return true;
}

void place(int r, int c, int len, bool vertical, vector<vector<Tile> > &grid){
    for(int i = 0; i < len; i++){
        if(vertical){
            //grid[r + i][c].changeColor(sf::Color::Red);
            if(i + 1 == len) grid[r + i][c].next = &grid[r][c];
            else grid[r + i][c].next = &grid[r + i + 1][c];
            grid[r + i][c].hasShip = true;
        } else{
            //grid[r][c + i].changeColor(sf::Color::Red);
            if(i + 1 == len) grid[r][c + i].next = &grid[r][c];
            else grid[r][c + i].next = &grid[r][c + i + 1];
            grid[r][c + i].hasShip = true;
        }
    }
    return;
}

void removeShip(int r, int c, int len, bool vertical, vector<vector<Tile> > &grid){
    for(int i = 0; i < len; i++){
        if(vertical){
            grid[r + i][c].changeColor(sf::Color::Blue);
            grid[r + i][c].hasShip = false;
        } else{
            grid[r][c + i].changeColor(sf::Color::Blue);
            grid[r][c + i].hasShip = false;

        }
    }
    return;
}

class Ship {
public:
    float x, y;
    bool isMouseOver;
    bool isDragging;
    bool isPlaced;
    bool vertical;
    int shipLen;
    std::vector<int> gridPos;
    
    Ship(float x1, float y1, int len) : grey(50, 50, 50), isMouseOver(false), isDragging(false), isPlaced(false), vertical(true), isPlacedVertical(true){
        originalWidth = 20;
        originalHeight = 30 * len - 5;

        rectangle.setSize(sf::Vector2f(originalWidth, originalHeight));
        rectangle.setPosition(x1, y1);
        origPos = sf::Vector2f(x1,y1);
        x = x1;
        y = y1;
        shipLen = len;
        rectangle.setFillColor(grey);
        rectangle.setOutlineColor(sf::Color::White);
        rectangle.setOutlineThickness(2);
        gridPos.resize(2);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(rectangle);
    }

    void update(sf::RenderWindow& window, sf::Event event, vector<vector<Tile> > &grid) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect bounds = rectangle.getGlobalBounds();

        if(isDragging){
            rectangle.setSize(sf::Vector2f(originalWidth * 1.2f, originalHeight * 1.2f));
            rectangle.setPosition(sf::Vector2f(mousePos.x-5,mousePos.y-5));
            dragOffsetX = static_cast<float>(mousePos.x) - rectangle.getPosition().x;
            dragOffsetY = static_cast<float>(mousePos.y) - rectangle.getPosition().y;
            if(isPlaced){
                isPlaced = false;
                removeShip(gridPos[0],gridPos[1],shipLen,vertical,grid);
            }
        } else if(!isPlaced) rectangle.setPosition(origPos);

        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            if(event.type == sf::Event::MouseButtonPressed){
                if(isDraggingGlobal){
                    for(int i = 0; i < 10; i++){
                        for(int j = 0; j < 10; j++){
                            sf::FloatRect tileBound = grid[i][j].getBound();
                            if(tileBound.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)) && canPlace(i,j,shipLen,vertical, grid)){
                                place(i, j, shipLen, vertical, grid);
                                isPlaced = true;
                                gridPos[0] = i; gridPos[1] = j;
                                rectangle.setPosition(grid[i][j].getPos());
                            }
                        }
                    }
                    isDraggingGlobal = false;
                    isDragging = false;
                    rectangle.setSize(sf::Vector2f(originalWidth, originalHeight));
                } else{
                    isDragging = true;
                    isDraggingGlobal = true;
                }
            }
            if(!isDraggingGlobal){
                rectangle.setSize(sf::Vector2f(originalWidth * 1.2f, originalHeight * 1.2f));
            }
            if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::R && isDragging){
                rectangle.setSize(sf::Vector2f(originalHeight,originalWidth));
                swap(originalWidth, originalHeight);
                
                rectangle.setPosition(sf::Vector2f(mousePos.x-5,mousePos.y-5));
                
                rectangle.setSize(sf::Vector2f(originalWidth * 1.2f, originalHeight * 1.2f));
                if(vertical){
                    vertical = false;
                    if(!isPlaced) isPlacedVertical = false;
                }
                else{
                    vertical = true;
                    if(!isPlaced) isPlacedVertical = true;                    
                }
            }
        } else rectangle.setSize(sf::Vector2f(originalWidth,originalHeight));
    }

    sf::Vector2f getPos() {
        return sf::Vector2f(x, y);
    }

private:
    sf::RectangleShape rectangle;
    sf::Color grey;
    sf::Vector2f origPos;
    float originalWidth, originalHeight;
    float dragOffsetX, dragOffsetY;
    bool isPlacedVertical;
};

bool makeTurn(sf::RenderWindow& window, vector<vector<Tile> > &grid1, vector<vector<Tile> > &grid2, vector<Ship> &ships, int &hp, sf::Sound &sound, sf::Sound &sound2, sf::Text text2){
    bool b = true;
    while(b) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if(!grid2[i][j].getShot(window, event, hp, grid2, sound, sound2)){
                        b = false;
                        return 1;
                    }
                    if(hp == 0) return 1;
                }
            }
        }
        window.clear(sf::Color::Black);
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                grid1[i][j].draw(window);
                grid2[i][j].draw(window);
            }
        }
        for (int i = 0; i < ships.size(); i++) {
            ships[i].draw(window);
        }
        window.draw(text2);
        window.display();
    }
    return 1;
}

bool printText(sf::RenderWindow &window, sf::Text text, int hp){
    if(hp == 0) return 1;
    sf::Event event;
    window.clear(sf::Color::Black);
    window.draw(text);
    window.display();
    bool press = 0;
    while (!press) {
        window.pollEvent(event);
        if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Enter) {
            press = 1;
        } else if(event.type == sf::Event::Closed) return 0;
    }
    return 1;
}

bool events(sf::RenderWindow &window){
    sf::Event event;
    bool press = 0;
    while(!press){
        window.pollEvent(event);
        if(event.type == sf::Event::Closed){
            return 1;
        } else if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Enter){
            return 0;
        }
    }
    return 0;
}

int main() {
    sf::Image background;
    sf::RenderWindow window(sf::VideoMode(864, 864), "SFML Window");
    window.setTitle("Battleships");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(30);

    sf::Font font;
    if (!font.loadFromFile(getFontPath("Capture it.ttf"))) {
        return 1;
    }
    sf::SoundBuffer buffer;
    if(!buffer.loadFromFile(getFontPath("explosion-80108.mp3"))){
        return 2;
    }
    sf::SoundBuffer buffer2;
    if(!buffer2.loadFromFile(getFontPath("splash.wav"))){
        return 3;
    }
    sf::Sound sound(buffer);
    sound.setVolume(40.0f);
    sf::Sound sound2(buffer2);
    
    sf::Music music;
    if (!music.openFromFile(getFontPath("oceansound.mp3"))) {
        return 4;
    }
    music.setLoop(true);
    music.play();
    sf::Text text("Press ENTER to start turn", font);
    sf::Text text2("Green - Miss\nWhite - Hit", font); 
    sf::Text text3("Press R to rotate ship", font);     
    text2.setPosition(2,400);
    text3.setPosition(2,400);
    text.setCharacterSize(30);
    bool game = true;
    while (game) {
        vector<vector<Tile> > grid1;
        vector<vector<Tile> > grid2;
        vector<Ship> ships1;
        vector<Ship> ships2;
        for(int i = 0; i < 10; i++){
            grid1.push_back({});
            for (int j = 0; j < 10; j++){
                grid1[i].push_back(Tile(110+(j*34), 20+(i*34), 30, i , j));
            }
        }
        for(int i = 0; i < 10; i++){
            grid2.push_back({});
            for (int j = 0; j < 10; j++){
                grid2[i].push_back(Tile(457+(j*34), 20+(i*34), 30, i , j));
            }
        }
        ships1.emplace_back(Ship(2,2,4));
        ships1.emplace_back(Ship(32,2,3));
        ships1.emplace_back(Ship(2,145,3));
        ships1.emplace_back(Ship(32,145,2));
        ships1.emplace_back(Ship(2,252,2));
        ships1.emplace_back(Ship(32,252,2));
        ships1.emplace_back(Ship(2,323,1));
        ships1.emplace_back(Ship(32,323,1));
        ships1.emplace_back(Ship(2,358,1));
        ships1.emplace_back(Ship(32,358,1));
        ships2.emplace_back(Ship(812,2,4));
        ships2.emplace_back(Ship(842,2,3));
        ships2.emplace_back(Ship(812,145,3));
        ships2.emplace_back(Ship(842,145,2));
        ships2.emplace_back(Ship(812,252,2));
        ships2.emplace_back(Ship(842,252,2));
        ships2.emplace_back(Ship(812,323,1));
        ships2.emplace_back(Ship(842,323,1));
        ships2.emplace_back(Ship(812,358,1));
        ships2.emplace_back(Ship(842,358,1));
        bool player1 = true;
        bool player2 = true;
        while (player1) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return 0;
                }
                for (int i = 0; i < ships1.size(); i++) {
                    ships1[i].update(window, event, grid1);
                }
            }

            int cnt = 0;
            for (int i = 0; i < ships1.size(); i++) {
                if (ships1[i].isPlaced) cnt++;
            }

            if (cnt == 10) {
                window.clear(sf::Color::Black);
                player1 = false;
            } else {
                window.clear(sf::Color::Black);
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 10; j++) {
                        grid1[i][j].draw(window);
                    }
                }
                for (int i = 0; i < ships1.size(); i++) {
                    ships1[i].draw(window);
                }
            }
            window.draw(text3);
            window.display();
        }
        while (player2) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                    return 0;
                }
                for (int i = 0; i < ships1.size(); i++) {
                    ships2[i].update(window, event, grid2);
                }
            }

            int cnt = 0;
            for (int i = 0; i < ships1.size(); i++) {
                if (ships2[i].isPlaced) cnt++;
            }

            if (cnt == 10) {
                window.clear(sf::Color::Black);
                player2 = false;
            } else {
                window.clear(sf::Color::Black);
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 10; j++) {
                        grid2[i][j].draw(window);
                    }
                }
                for (int i = 0; i < ships1.size(); i++) {
                    ships2[i].draw(window);
                }
            }
            window.draw(text3);
            window.display();
        }
        window.clear(sf::Color::Black);
        int hp1 = 20;
        int hp2 = 20;
        bool turn = true;
        while (hp2 > 0 && hp1 > 0) {
            window.clear(sf::Color::Black);
            if(turn){
                if(makeTurn(window, grid1, grid2, ships1, hp2, sound, sound2, text2) == false){
                    return 0;
                }
                if(!printText(window,text, hp2)) return 0;
            } else{
                if(makeTurn(window, grid2, grid1, ships2, hp1, sound, sound2, text2) == false){
                    return 0;
                }
                if(!printText(window,text,hp1)) return 0;
            }
            turn = !turn;
        }
        window.clear(sf::Color::Black);
        if(hp1 > hp2){
            sf::Text winner("Player 1 is the winner", font);
            window.draw(winner); 
            window.display();
            if(events(window)) game = false;
        } else{
            sf::Text winner("Player 2 is the winner", font);
            window.draw(winner);
            window.display();
            if(events(window)) return 0;
        }
    }
    return 0;
}



















