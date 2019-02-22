#include <string>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;

struct Coord {

	int x;
	int y;

	bool operator == (Coord &other) {
	
		return this->x == other.x && this->y == other.y;
	}
};

class AssetReader {

private:
	ifstream in;

public:

	void readFileWithUnknownSize(vector<string> &ls, string file) {

		in.open(file);
		int height, width;
		in >> noskipws >> height;
		in >> noskipws >> width;
		in.close();
		readFileIntoVectorString(ls, file, height, width);
	}
	void readFileIntoVectorString(vector<string> &ls, string file, int height, int width) {
	
		in.open(file);
		ls.clear();
		ls.resize(height);
		string s = "";
		char c;
		for (int i = 0; i < (int)ls.size(); i++) {
		
			for (int j = 0; j < width; j++) {
			
				in >> noskipws >> c;
				s += c;
			}	
			ls[i] = s;
			s = "";
		}
		in.close();
	}
};

class Dinosaur {

public:
	static const int height = 9, width = 16;
private:
	Coord pivot;
	Coord org_pivot;
	vector<string> body_idle;
	vector<string> body_run1;
	vector<string> body_run2;
	vector<string> body;

	const string asset_idle = "dino_idle.txt";
	const string asset_run1 = "dino_run1.txt";
	const string asset_run2 = "dino_run2.txt";

	int state_code = IDLE_STATE;
	int on_space = 0;
	int max_on_space = 2;

	AssetReader assetReader;
	void readAssetFile() {
	
		assetReader.readFileIntoVectorString(body_idle, asset_idle, height, width);
		assetReader.readFileIntoVectorString(body_run1, asset_run1, height, width);
		assetReader.readFileIntoVectorString(body_run2, asset_run2, height, width);
	}

public:

	static const int IDLE_STATE = 100;
	static const int RUN1_STATE = 111;
	static const int RUN2_STATE = 122;
	static const int JUMP_STATE = 133;
	static const int FALL_STATE = 144;
	bool start = true;

	Dinosaur(int y, int x) {
	
		this->pivot.x = x;
		this->pivot.y = y;
		readAssetFile();
		body = body_idle;
		org_pivot = pivot;
		pivot.x = 0;
	}
	
	Coord getPivot() {
	
		return this->pivot;
	}
	void setPivotY(int y) {
	
		this->pivot.y = y;
	}
	
	int getHeight() {
	
		return this->height;
	}
	int getWidth() {
	
		return this->width;
	}

	bool onRightPos() {
	
		return pivot.x == org_pivot.x;
	}

	void setMaxOnSpace(int t) {
	
		this->max_on_space = t;
	}

	void Start() {
	
		Jump();
		pivot.x = 0;
		start = true;
	}

	void Jump() {

		if (this->state_code != FALL_STATE && this->state_code != JUMP_STATE) setStateCode(JUMP_STATE), on_space = 0;
	}
	
	void Rise() {

		if (this->state_code == JUMP_STATE) {
		
			if (pivot.y <= org_pivot.y - height * 2) {
			
				pivot.y = org_pivot.y - height * 2;
				if (on_space == max_on_space) this->state_code = FALL_STATE, on_space = 0;
				else on_space++;
			}
			else pivot.y--;
		}
	}
	
	void Fall() {
	
		if (this->state_code == FALL_STATE) {
		
			if (pivot.y >= org_pivot.y) {
			
				pivot.y = org_pivot.y;
				this->state_code = RUN1_STATE;
			}
			else pivot.y++;
		}
		if ((state_code == RUN1_STATE || state_code == RUN2_STATE) && pivot.x < org_pivot.x) pivot.x++;
	}

	void toggleStep() {
	
		if (state_code == RUN1_STATE) setStateCode(RUN2_STATE);
		else if (state_code == RUN2_STATE) setStateCode(RUN1_STATE);
	}

	int getStateCode() {
	
		return this->state_code;
	}

	void setStateCode(int code) {
	
		this->state_code = code;

		if (state_code == IDLE_STATE) body = body_idle;
		if (state_code == JUMP_STATE) body = body_idle;
		if (state_code == FALL_STATE) body = body_idle;
		if (state_code == RUN1_STATE) body = body_run1;
		if (state_code == RUN2_STATE) body = body_run2;
	}

	void Draw() {
	
		for (int i = 0; i < height; i++) {
		
			mvaddstr(pivot.y + i, pivot.x, const_cast<char *>(body[i].c_str()));
		}
	}

	vector<string> getBody() {
	
		return this->body;
	}
};

class Obstacle {

private:
	vector<string> body;
	string asset_file;
	Coord pivot;
	int height, width;

	void readAssetFile() {
	
		AssetReader assetReader;
		assetReader.readFileIntoVectorString(body, asset_file, height, width);
	}

public:

	Obstacle();
	Obstacle(string asset_file, int x, int y) {
	
		this->asset_file = asset_file;
		this->pivot.x = x;
		this->pivot.y = y;
	}
	
	string getAssetFile() {
	
		return this->asset_file;
	}
	void setAssetFile(string file) {
	
		this->asset_file = file;
	}
	vector<string> getBody() {
	
		return this->body;
	}
};

class Land {

public:
	int length;
	string layer0;
	string layer1;
	string layer2;
	
	int pos_line;
	int curr = 1;

public:

	void makeLayer(bool df = true, int dfl = 10){

		if (df) dfl = length;

		layer0 = "";
		int odd_count = 2;
		int c = 0;
		for (int i = 0; i < dfl; i++) { 
			int rd = rand() % 40;
			layer0 += (c < odd_count && rd == 0)? "^": "=";
			c += rd == 0;
		}

		int dot_count = dfl / 6;
		layer1 = "";
		layer2 = "";
		for (int i = 0; i < dfl; i++) {
		
			int rd = rand() % 8;
			layer1 += (rd == 0 && c < dot_count)? ".":" ";
			c += rd == 0;
		}
		c = 0;
		dot_count = dfl / 10;
		for (int i = 0; i < dfl; i++) {
		
			int rd = rand() % 8;
			layer2 += (rd == 0 && c < dot_count)? ".":" ";
			c += rd == 0;
		}
	}

	void Move() {
	
		if (curr == 2*length) { makeLayer(); curr = 1; }
		char temp = layer1[0]; layer1 = layer1.substr(1); layer1 += temp;
		temp = layer2[0]; layer2 = layer2.substr(1); layer2 += temp;
		temp = layer0[0]; layer0 = layer0.substr(1); layer0 += temp;
		curr++;	
	}
	void Draw() {
	
		mvaddstr(pos_line, 0, const_cast<char *>(layer0.c_str()));
		mvaddstr(pos_line + 1, 0, const_cast<char *>(layer1.c_str()));
		mvaddstr(pos_line + 2, 0, const_cast<char *>(layer2.c_str()));
	}

	void setPos(int pos) {
	
		this->pos_line = pos;
	}

	Land() {
	
		makeLayer(false, length / 10);
	}
	Land(int length, int pos_line) {
	
		this->length = length;
		this->pos_line = pos_line;
		makeLayer(false, length / 10);
	}
};

class GameManager {

private:
	int map_width, map_height;
	int highscore = 0;
	int score = 0;

	Land *land;
	Dinosaur *dino;
	vector<Obstacle> obs_list;
	vector<string> map, blank_map; 
	
	const string highscore_file = "highscore.txt";
	bool keyPressed = false;
	int beat = 0;
	int difficult = 0;
	int clock = 10000;
	time_t t_start, t_now;

public:

	bool isGameOver = false;
	GameManager();
	GameManager(int height, int width) {

		this->map_height = height;
		this->map_width = width;
		string s(map_width, ' ');
		blank_map.clear();
		for (int i = 0; i < map_height; i++) blank_map.push_back(s);

	}
	~GameManager() {
	
		delete dino;
		delete land;
	}

	void changeState() {
	
		time(&t_now);
		land->Move();
		if (beat % 8 == 0) dino->toggleStep();
		if (beat % 2 == 0) dino->Rise();
		if (beat % 2 == 0 ) dino->Fall();
		
		if (difftime(t_now, t_start) >= 10) {
			difficult++;
			difficult = min(8, difficult);
			time(&t_start);
		}
	
		if (getch() == ' ' && !dino->start) {
			dino->Jump();
			
		}
		if (dino->start && dino->onRightPos()) land->makeLayer(), dino->start = false;;
		beat++;

		dino->setMaxOnSpace(2 + min(6, difficult));
		clock = max(2000, 10000 - difficult*800);
	}

	void setDifficult(int diff) {
	
		this->difficult = diff;
	}

	void calcPos() {
	
		for (int j = 0; j < map_width; j++) {
		
			map[map_height - 3][j] = land->layer0[j];
			map[map_height - 2][j] = land->layer1[j];
			map[map_height - 1][j] = land->layer2[j];
		}

		for (int i = 0; i < dino->getHeight(); i++) {
		
			for (int j = 0; j < dino->getWidth(); j++) {
			
				map[dino->getPivot().y + i][dino->getPivot().x + j] = dino->getBody()[i][j];
			}
		}
	} 

	void Render() {

	    map = blank_map;		
		calcPos();
		
		for (int i = 0; i < map_height; i++) {
			mvaddstr(i, 0, const_cast<char *> (map[i].c_str()));
		}
		if (dino->getStateCode() == Dinosaur::JUMP_STATE) mvaddstr(0, 0, "is rising");
		else if (dino->getStateCode() == Dinosaur::FALL_STATE) mvaddstr(0, 0, "is falling");
		move(1, 0); printw("%d", difficult);
	}

	void Init() {
		
		dino = new Dinosaur(map_height - Dinosaur::height - 3, 10);
		land = new Land(map_width, map_height);
		score = 0;
		isGameOver = true;

		Render();
		nodelay(stdscr, FALSE);
		if (getch() == ' ') dino->Start();	
		time(&t_start);
		nodelay(stdscr, TRUE);
	}

	void Play() {
		
		Init();
		while(true) {
			
			Render();	
			changeState();
			usleep(clock);
			refresh();
		}
	}
};

int main() {

	initscr();	
	curs_set(0);
	cbreak();
    noecho();
    scrollok(stdscr, TRUE);
   	nodelay(stdscr, TRUE);
	
	GameManager gameManager(50, 200);
	gameManager.Play();
	getch();
	endwin();
	return 0;
}
