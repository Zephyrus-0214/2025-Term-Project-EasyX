#include<graphics.h>
#include<string>
#include<vector>
#include<math.h>

using namespace std;

const double pi = 3.14159;

int idx_current_anim = 0;  //当前帧索引
const int PLAYER_ANIM_NUM = 6;  //玩家动画总帧数

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];

#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

bool running = true;
bool is_game_started = false;  //标记游戏是否已经开始

//借助系统绘图函数实现透明通道混叠
inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

void LoadAnimation()
{
	for (size_t i = 0;i < PLAYER_ANIM_NUM;i++)
	{
		wstring path = L"img/player_left_" + to_wstring(i) + L".png";
		loadimage(&img_player_left[i], path.c_str());
	}

	for (size_t i = 0;i < PLAYER_ANIM_NUM;i++)
	{
		wstring path = L"img/player_right_" + to_wstring(i) + L".png";
		loadimage(&img_player_right[i], path.c_str());
	}
}

class Atlas  //动画所使用的图集
{
public:
	Atlas(LPCTSTR path,int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0;i < num;i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();  //注意内存管理
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		for (size_t i = 0;i < frame_list.size();i++)
			delete frame_list[i];  //释放内存
	}

public:
	vector<IMAGE*>frame_list; //容器 动态分配内存

};

Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;

//封装类 解决代码冗余
class Animation
{
public:
	Animation(Atlas* atlas, int interval)
	{
		anim_atlas = atlas;
		interval_ms = interval;
	}

	~Animation() = default;

	void Play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	}

private:
	int timer = 0;   // 动画计时器
	int idx_frame = 0;   // 动画帧索引
	int interval_ms = 0;

private:
	Atlas* anim_atlas;
};

//玩家类
class Player
{
public:
	const int PLAYER_WIDTH = 68;  //玩家宽度
	const int PLAYER_HEIGHT = 108;  //玩家高度
	//作为全局变量方便检测与敌人碰撞时访问以上两个数据

	Player()
	{
		anim_left = new Animation(atlas_player_left, 45);
		anim_right = new Animation(atlas_player_right, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_KEYDOWN:
			switch (msg.vkcode)
			{
			case VK_UP:
				//不直接对玩家位置数据操作，以设置布尔变量的值代替
				is_move_up = true;
				break;
			case VK_DOWN:
				is_move_down = true;
				break;
			case VK_LEFT:
				is_move_left = true;
				break;
			case VK_RIGHT:
				is_move_right = true;
				break;
			}
			break;

		case WM_KEYUP:
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = false;
				break;
			case VK_DOWN:
				is_move_down = false;
				break;
			case VK_LEFT:
				is_move_left = false;
				break;
			case VK_RIGHT:
				is_move_right = false;
				break;
			}
			break;
		}
	}

	void Move()
	{
		//解决斜向移动时速度叠加的问题
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_pos.x += (int)(PLAYER_SPEED * normalized_x);
			player_pos.y += (int)(PLAYER_SPEED * normalized_y);
		}

		//限制玩家在窗口内移动
		if (player_pos.x < 0)
			player_pos.x = 0;
		if (player_pos.y < 0)
			player_pos.y = 0;
		if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH)
			player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT)
			player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
	}

	void Draw(int delta)
	{
		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left->Play(player_pos.x, player_pos.y, delta);
		else
			anim_right->Play(player_pos.x, player_pos.y, delta);
	}

	const POINT& GetPosition() const //获取玩家位置，在敌人类中会用到
	{
		return player_pos;
	}

private:
	const int PLAYER_SPEED = 3;   //玩家移动速度

private:
	Animation* anim_left;
	Animation* anim_right;
	POINT player_pos = { 500,500 };  //玩家坐标

	//玩家是否向对应方向移动(解决动画不连贯等问题）
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
};

//粒子类
class Particle
{
public:
	POINT position;
	int lifetime; // 粒子寿命
	COLORREF color;

	Particle(POINT pos, COLORREF col, int life)
		: position(pos), color(col), lifetime(life) {
	}

	void Update()
	{
		position.x += rand() % 5 - 2; // 随机移动
		position.y += rand() % 5 - 2;
		lifetime--;
	}

	void Draw() const
	{
		setfillcolor(color);
		solidcircle(position.x, position.y, 2); // 绘制粒子
	}

	bool IsAlive() const { return lifetime > 0; }
};

//子弹类
class Bullet
{
public:
	POINT position = { 0,0 };
	static IMAGE img_basketball;
	vector<Particle> particles; // 粒子列表

	static void LoadBasketballImage()
	{
		loadimage(&img_basketball, _T("img/basketball.png"));
	}

	void UpdateEffect()
	{
		// 添加新粒子
		for (int i = 0; i < 2; i++)//每2帧生成一个粒子
		{
			particles.emplace_back(position, RGB(186, 85, 211), 20); //寿命30帧
		}

		// 更新粒子
		for (auto& particle : particles)
		{
			particle.Update();
		}

		// 移除粒子
		particles.erase(remove_if(particles.begin(), particles.end(),
			[](const Particle& p) { return !p.IsAlive(); }),
			particles.end());
	}

	void DrawEffect() const
	{
		for (const auto& particle : particles)
		{
			particle.Draw();
		}
	}

	void Draw() const
	{
		putimage_alpha(position.x - img_basketball.getwidth() / 2,
			position.y - img_basketball.getheight() / 2,
			&img_basketball);
	}
};

IMAGE Bullet::img_basketball;

//敌人类
class Enemy
{
public:
	Enemy()
	{
		anim_left = new Animation(atlas_enemy_left, 45);
	    anim_right= new Animation(atlas_enemy_right, 45);

		//敌人生成边界
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//将敌人放置在地图外边界处的随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -ENEMY_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -ENEMY_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		default:
			break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		//将子弹等效为点，判断点是否在敌人所处的矩形内部
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + ENEMY_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + ENEMY_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		//将敌人中心位置等效为点，判断点是否在玩家所处的矩形内部
		POINT check_position = { position.x + ENEMY_WIDTH / 2,position.y + ENEMY_HEIGHT / 2 };  //找到敌人所处矩形的中心点
		bool is_overlap_x = check_position.x >= player.GetPosition().x && check_position.x <= player.GetPosition().x + player.PLAYER_WIDTH;
		bool is_overlap_y = check_position.y >= player.GetPosition().y && check_position.y <= player.GetPosition().y + player.PLAYER_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(ENEMY_SPEED * normalized_x);
			position.y += (int)(ENEMY_SPEED * normalized_y);
		}
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}

	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

	void Draw(int delta)
	{
		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	const POINT& GetPosition() const
	{
		return position;
	}

	void Hurt()
	{
		alive = false;  //一击必杀
	}

	//检测敌人是否存活
	bool CheckAlive()  
	{
		return alive;
	}

private:
	const int ENEMY_WIDTH = 128;  //敌人宽度
	const int ENEMY_HEIGHT = 72;   //敌人高度
	const int ENEMY_SPEED = 2;  //敌人移动速度

private:
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
	bool alive = true;
};

//按钮类
class Button
{
public:
	Button(RECT rect,LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect;
		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	~Button() = default;

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
				OnClick();
			break;
		default:
			break;
		}
	}

	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;  //纯虚函数

private:
	enum class Status  //当前状态枚举变量
	{
		Idle=0,
		Hovered,
		Pushed
	};

private:
	RECT region;  //描述按钮位置与大小
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;

private:
	//检测鼠标点击
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

//开始游戏按钮
class StartGameButton :public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~StartGameButton() = default;

protected:
	void OnClick()
	{
		is_game_started = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);  //播放背景音乐
	}
};

//退出游戏按钮
class QuitGameButton :public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~QuitGameButton() = default;

protected:
	void OnClick()
	{
		running = false;
	}
};

//生成新的敌人
int enemy_speed = 400;  //调整敌人生成速度的参数
void TryGenerateEnemy(vector<Enemy*>& enemy_list)
{
	static int counter = 0;
	if ((++counter) % enemy_speed == 0)
	{
		Enemy* new_enemy = new Enemy();
		enemy_list.push_back(new_enemy);
		// 打印生成位置以调试
		printf("Generated enemy at (%d, %d)\n", new_enemy->GetPosition().x, new_enemy->GetPosition().y);
	}
}

//更新子弹位置
void UpdateBullets(vector<Bullet>& bullet_list, const Player& player)
{
	const double NORMAL_SPEED = 0.0040;  //法向速度
	const double TANGENTIAL_SPEED = 0.0037;   //切向速度
	double radian_interval = 2 * pi / bullet_list.size();  //子弹之间的弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * NORMAL_SPEED);
	for (size_t i = 0;i < bullet_list.size();i++)
	{
		double radian = GetTickCount() * TANGENTIAL_SPEED + radian_interval * i;  //子弹当前所在弧度值
		bullet_list[i].position.x = player_position.x + player.PLAYER_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.PLAYER_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}

//绘制玩家得分
void DrawPlayerScore(int score)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分:%d"),score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(0, 0, 0));
	outtextxy(10, 10, text);
}

bool show_enemy_level_message = false;
DWORD message_start_time = 0;

int main()
{
	atlas_player_left = new Atlas(_T("img/player_left_%d.png"), 6);
	atlas_player_right = new Atlas(_T("img/player_right_%d.png"), 6);
	atlas_enemy_left = new Atlas(_T("img/enemy_left_%d.png"), 6);
	atlas_enemy_right = new Atlas(_T("img/enemy_right_%d.png"), 6);

	int score = 0;
	Player player;
	vector<Enemy*> enemys;
	
	IMAGE img_background;
	loadimage(&img_background, _T("img/background.png"));
	IMAGE img_menu;
	loadimage(&img_menu, _T("img/menu.png"));
	Bullet::LoadBasketballImage();

	mciSendString(_T("open music/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open music/hit.mp3 alias hit"), NULL, 0, NULL);

	initgraph(1280, 720);
	BeginBatchDraw();
	ExMessage msg;
	vector<Enemy*>enemy_list;
	int bullet_num = 1;
	vector<Bullet>bullet_list(bullet_num);

	RECT region_btn_start_game, region_btn_quit_game;
	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;
	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = (region_btn_quit_game.left + BUTTON_WIDTH);
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;
	
	StartGameButton btn_start_game = StartGameButton(region_btn_start_game, _T("img/ui_start_idle.png"), _T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game, _T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	while (running)
	{
		DWORD startTime = GetTickCount();
		TryGenerateEnemy(enemys);
		//获取输入
		while (peekmessage(&msg))
		{
			if (is_game_started)
				player.ProcessEvent(msg);
			else
			{
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}

		if (is_game_started)
		{
			//数据处理
			player.Move();
			UpdateBullets(bullet_list, player);
			TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
				enemy->Move(player);

			//检测敌人与玩家的碰撞
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(player))
				{
					static TCHAR text[128];
					_stprintf_s(text, _T("最终得分:%d !"), score);
					MessageBox(GetHWnd(), text, _T("游戏结束"), MB_OK);
					running = false;
					break;
				}
			}

			//检测敌人被子弹攻击
			for (Enemy* enemy : enemy_list)
			{
				for (const Bullet& bullet : bullet_list)
				{
					if (enemy->CheckBulletCollision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						enemy->Hurt();
						score++;
					}
				}
				int N1 = 15;
				static int last_player_Lv = 0; // 记录上次的玩家等级
				int current_player_Lv = score / N1; // 当前玩家等级
				if (current_player_Lv > last_player_Lv)
				{
					bullet_num += 1;  // 每击杀 15 个敌人增加 1 个子弹
					bullet_list.resize(bullet_num); // 重置子弹数量
					last_player_Lv = current_player_Lv; // 更新玩家等级

					MessageBox(GetHWnd(), _T("你的等级已提升，篮球+1"), _T("等级提升"), MB_OK);
				}
				
				int N2 = 25;
				static int last_enemy_Lv = 0; // 记录上次的敌人等级
				int current_enemy_Lv = score / N2; // 当前敌人等级
				if (current_enemy_Lv > last_enemy_Lv)
				{
					enemy_speed -= 20;  // 每次减少固定值
					if (enemy_speed < 60)
					{
						enemy_speed = 60;  // 防止 enemy_speed 小于等于 0
					}
					last_enemy_Lv = current_enemy_Lv; // 更新敌人等级

					show_enemy_level_message = true;
					message_start_time = GetTickCount();
				}

				if (show_enemy_level_message)
				{
					DWORD current_time = GetTickCount();
					if (current_time - message_start_time <= 1000)  //停留一秒
					{
						setbkmode(TRANSPARENT);
						settextcolor(RGB(255, 0, 0));
						int text_x = (WINDOW_WIDTH - textwidth(_T("敌人等级增加！"))) / 2;
						int text_y = 20; 
						outtextxy(text_x, text_y, _T("敌人等级增加！"));
					}
					else
					{
						show_enemy_level_message = false;
					}
				}
			}

			//移除受击敌人
			for (size_t i = 0;i < enemy_list.size();i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;  //避免内存泄露
				}
			}
		}
		
		cleardevice();
		if (is_game_started)
		{
			putimage(0, 0, &img_background);
			player.Draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
			{
				enemy->Draw(1000 / 144);
			}
			for (Bullet& bullet : bullet_list)
			{
				bullet.UpdateEffect();  // 更新粒子效果
				bullet.DrawEffect();  // 绘制粒子效果
				bullet.Draw(); 
			}
			DrawPlayerScore(score);
		}
		else
		{
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}
		
		FlushBatchDraw();

		DWORD endTime = GetTickCount();
		DWORD deltaTime = endTime - startTime;
		if (deltaTime < 1000 / 144)
		{
			Sleep(1000 / 144 - deltaTime);
		}
	}

	delete atlas_player_left;
	delete atlas_player_right;
	delete atlas_enemy_left;
	delete atlas_enemy_right;

	EndBatchDraw();

	return 0;
}