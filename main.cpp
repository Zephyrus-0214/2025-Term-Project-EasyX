#include<graphics.h>
#include<string>
#include<vector>
#include<math.h>

using namespace std;

const double pi = 3.14159;

int idx_current_anim = 0;  //��ǰ֡����
const int PLAYER_ANIM_NUM = 6;  //��Ҷ�����֡��

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];

#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

bool running = true;
bool is_game_started = false;  //�����Ϸ�Ƿ��Ѿ���ʼ

//����ϵͳ��ͼ����ʵ��͸��ͨ�����
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

class Atlas  //������ʹ�õ�ͼ��
{
public:
	Atlas(LPCTSTR path,int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0;i < num;i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();  //ע���ڴ����
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		for (size_t i = 0;i < frame_list.size();i++)
			delete frame_list[i];  //�ͷ��ڴ�
	}

public:
	vector<IMAGE*>frame_list; //���� ��̬�����ڴ�

};

Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;

//��װ�� �����������
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
	int timer = 0;   // ������ʱ��
	int idx_frame = 0;   // ����֡����
	int interval_ms = 0;

private:
	Atlas* anim_atlas;
};

//�����
class Player
{
public:
	const int PLAYER_WIDTH = 68;  //��ҿ��
	const int PLAYER_HEIGHT = 108;  //��Ҹ߶�
	//��Ϊȫ�ֱ����������������ײʱ����������������

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
				//��ֱ�Ӷ����λ�����ݲ����������ò���������ֵ����
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
		//���б���ƶ�ʱ�ٶȵ��ӵ�����
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

		//��������ڴ������ƶ�
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

	const POINT& GetPosition() const //��ȡ���λ�ã��ڵ������л��õ�
	{
		return player_pos;
	}

private:
	const int PLAYER_SPEED = 3;   //����ƶ��ٶ�

private:
	Animation* anim_left;
	Animation* anim_right;
	POINT player_pos = { 500,500 };  //�������

	//����Ƿ����Ӧ�����ƶ�(�����������������⣩
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
};

//������
class Particle
{
public:
	POINT position;
	int lifetime; // ��������
	COLORREF color;

	Particle(POINT pos, COLORREF col, int life)
		: position(pos), color(col), lifetime(life) {
	}

	void Update()
	{
		position.x += rand() % 5 - 2; // ����ƶ�
		position.y += rand() % 5 - 2;
		lifetime--;
	}

	void Draw() const
	{
		setfillcolor(color);
		solidcircle(position.x, position.y, 2); // ��������
	}

	bool IsAlive() const { return lifetime > 0; }
};

//�ӵ���
class Bullet
{
public:
	POINT position = { 0,0 };
	static IMAGE img_basketball;
	vector<Particle> particles; // �����б�

	static void LoadBasketballImage()
	{
		loadimage(&img_basketball, _T("img/basketball.png"));
	}

	void UpdateEffect()
	{
		// ���������
		for (int i = 0; i < 2; i++)//ÿ2֡����һ������
		{
			particles.emplace_back(position, RGB(186, 85, 211), 20); //����30֡
		}

		// ��������
		for (auto& particle : particles)
		{
			particle.Update();
		}

		// �Ƴ�����
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

//������
class Enemy
{
public:
	Enemy()
	{
		anim_left = new Animation(atlas_enemy_left, 45);
	    anim_right= new Animation(atlas_enemy_right, 45);

		//�������ɱ߽�
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//�����˷����ڵ�ͼ��߽紦�����λ��
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
		//���ӵ���ЧΪ�㣬�жϵ��Ƿ��ڵ��������ľ����ڲ�
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + ENEMY_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + ENEMY_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		//����������λ�õ�ЧΪ�㣬�жϵ��Ƿ�����������ľ����ڲ�
		POINT check_position = { position.x + ENEMY_WIDTH / 2,position.y + ENEMY_HEIGHT / 2 };  //�ҵ������������ε����ĵ�
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
		alive = false;  //һ����ɱ
	}

	//�������Ƿ���
	bool CheckAlive()  
	{
		return alive;
	}

private:
	const int ENEMY_WIDTH = 128;  //���˿��
	const int ENEMY_HEIGHT = 72;   //���˸߶�
	const int ENEMY_SPEED = 2;  //�����ƶ��ٶ�

private:
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
	bool alive = true;
};

//��ť��
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
	virtual void OnClick() = 0;  //���麯��

private:
	enum class Status  //��ǰ״̬ö�ٱ���
	{
		Idle=0,
		Hovered,
		Pushed
	};

private:
	RECT region;  //������ťλ�����С
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;

private:
	//��������
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

//��ʼ��Ϸ��ť
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
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);  //���ű�������
	}
};

//�˳���Ϸ��ť
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

//�����µĵ���
int enemy_speed = 400;  //�������������ٶȵĲ���
void TryGenerateEnemy(vector<Enemy*>& enemy_list)
{
	static int counter = 0;
	if ((++counter) % enemy_speed == 0)
	{
		Enemy* new_enemy = new Enemy();
		enemy_list.push_back(new_enemy);
		// ��ӡ����λ���Ե���
		printf("Generated enemy at (%d, %d)\n", new_enemy->GetPosition().x, new_enemy->GetPosition().y);
	}
}

//�����ӵ�λ��
void UpdateBullets(vector<Bullet>& bullet_list, const Player& player)
{
	const double NORMAL_SPEED = 0.0040;  //�����ٶ�
	const double TANGENTIAL_SPEED = 0.0037;   //�����ٶ�
	double radian_interval = 2 * pi / bullet_list.size();  //�ӵ�֮��Ļ��ȼ��
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * NORMAL_SPEED);
	for (size_t i = 0;i < bullet_list.size();i++)
	{
		double radian = GetTickCount() * TANGENTIAL_SPEED + radian_interval * i;  //�ӵ���ǰ���ڻ���ֵ
		bullet_list[i].position.x = player_position.x + player.PLAYER_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.PLAYER_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}

//������ҵ÷�
void DrawPlayerScore(int score)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("��ǰ��ҵ÷�:%d"),score);

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
		//��ȡ����
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
			//���ݴ���
			player.Move();
			UpdateBullets(bullet_list, player);
			TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
				enemy->Move(player);

			//����������ҵ���ײ
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(player))
				{
					static TCHAR text[128];
					_stprintf_s(text, _T("���յ÷�:%d !"), score);
					MessageBox(GetHWnd(), text, _T("��Ϸ����"), MB_OK);
					running = false;
					break;
				}
			}

			//�����˱��ӵ�����
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
				static int last_player_Lv = 0; // ��¼�ϴε���ҵȼ�
				int current_player_Lv = score / N1; // ��ǰ��ҵȼ�
				if (current_player_Lv > last_player_Lv)
				{
					bullet_num += 1;  // ÿ��ɱ 15 ���������� 1 ���ӵ�
					bullet_list.resize(bullet_num); // �����ӵ�����
					last_player_Lv = current_player_Lv; // ������ҵȼ�

					MessageBox(GetHWnd(), _T("��ĵȼ�������������+1"), _T("�ȼ�����"), MB_OK);
				}
				
				int N2 = 25;
				static int last_enemy_Lv = 0; // ��¼�ϴεĵ��˵ȼ�
				int current_enemy_Lv = score / N2; // ��ǰ���˵ȼ�
				if (current_enemy_Lv > last_enemy_Lv)
				{
					enemy_speed -= 20;  // ÿ�μ��ٹ̶�ֵ
					if (enemy_speed < 60)
					{
						enemy_speed = 60;  // ��ֹ enemy_speed С�ڵ��� 0
					}
					last_enemy_Lv = current_enemy_Lv; // ���µ��˵ȼ�

					show_enemy_level_message = true;
					message_start_time = GetTickCount();
				}

				if (show_enemy_level_message)
				{
					DWORD current_time = GetTickCount();
					if (current_time - message_start_time <= 1000)  //ͣ��һ��
					{
						setbkmode(TRANSPARENT);
						settextcolor(RGB(255, 0, 0));
						int text_x = (WINDOW_WIDTH - textwidth(_T("���˵ȼ����ӣ�"))) / 2;
						int text_y = 20; 
						outtextxy(text_x, text_y, _T("���˵ȼ����ӣ�"));
					}
					else
					{
						show_enemy_level_message = false;
					}
				}
			}

			//�Ƴ��ܻ�����
			for (size_t i = 0;i < enemy_list.size();i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;  //�����ڴ�й¶
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
				bullet.UpdateEffect();  // ��������Ч��
				bullet.DrawEffect();  // ��������Ч��
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