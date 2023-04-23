// con_snake.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// 简陋的贪吃蛇游戏。
// include
#include <windows.h>
#include <stdio.h>

// 全局变量
// 控制颜色
WORD g_move_attr = BACKGROUND_GREEN | BACKGROUND_RED;
WORD g_ground_attr = 0;
WORD g_head_attr = FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE | BACKGROUND_RED;
WORD g_box_attr = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED;
WORD g_wall_attr = BACKGROUND_BLUE;
// 蛇身点集
COORD* g_snake = NULL;
int g_snake_len;
// 控制移动速度
double g_snake_speed = 1.0;//每秒一格
LONGLONG lasttime = GetTickCount64();
// 按键码 上下左右
WORD g_vkcode;
// 
CONSOLE_SCREEN_BUFFER_INFO g_csbi;
COORD g_newpt;
COORD g_boxpt = { -1,-1 };
int g_box_score = 0;
SMALL_RECT g_small_rect; // 限制刷新区域 
COORD g_size; //屏幕绘制区域大小
BOOL g_run = true;
CONSOLE_CURSOR_INFO g_cursor_info; // 保持光标信息

HANDLE g_istd = GetStdHandle(STD_INPUT_HANDLE);// 控制台输入句柄
HANDLE g_ostd = GetStdHandle(STD_OUTPUT_HANDLE); // 控制台输出句柄

/// <summary>
/// 显示分数
/// </summary>
/// <param name=""></param>
void ShowInfo(void)
{
	WORD attr;
	DWORD nLength;
	COORD coord;
	DWORD written;
	DWORD length;
	;
	attr = /*csbi.wAttributes |*/ FOREGROUND_GREEN | FOREGROUND_INTENSITY;

	coord = { SHORT(g_size.X / 4) , 2 };
	TCHAR str[32];
	swprintf_s(str, 32, TEXT("SCORE:%d"), g_box_score);
	length = lstrlen(str);
	SetConsoleCursorPosition(g_ostd, coord);
	if (!WriteConsole(g_ostd, str, length, &written, NULL))
		return;

	if (!FillConsoleOutputAttribute(g_ostd,         // Handle to console screen buffer
		attr, //csbi.wAttributes, // Character attributes to use
		length,        // Number of cells to set attribute
		coord,      // Coordinates of first cell
		&written))  // Receive number of characters written
	{
		return;
	}

	coord.Y += 1;
	swprintf_s(str, 32, TEXT("SPEED:%.1lf/S"), g_snake_speed);
	length = lstrlen(str);
	SetConsoleCursorPosition(g_ostd, coord);
	if (!WriteConsole(g_ostd, str, length, &written, NULL))
		return;

	if (!FillConsoleOutputAttribute(g_ostd,         // Handle to console screen buffer
		attr, //csbi.wAttributes, // Character attributes to use
		length,        // Number of cells to set attribute
		coord,      // Coordinates of first cell
		&written))  // Receive number of characters written
	{
		return;
	}
}
/// <summary>
/// 用于当吃掉box后蛇身加长1格
/// </summary>
/// <param name="new_pt">box坐标</param>
/// <param name="pts">蛇身点集</param>
/// <param name="len">点集长度</param>
void ExpandPT(COORD new_pt, COORD** pts, int* len)
{
	COORD* new_pts = new COORD[*len + 1];
	memcpy(new_pts, *pts, sizeof(COORD) * (*len));
	new_pts[*len] = new_pt;
	delete[] * pts;
	*pts = new_pts;
	*len += 1;
}

/// <summary>
/// 控制台绘制点集
/// </summary>
/// <param name="ostd">控制台</param>
/// <param name="size">缓冲区的大小</param>
/// <param name="pt">绘制的点集</param>
/// <param name="len">点集大小</param>
/// <param name="attr">绘制属性（颜色）</param>
/// <param name="update">是否绘制</param>
/// <returns>1 存在碰撞</returns>
DWORD ConsoleDraw(HANDLE ostd, COORD size, COORD* pt, int len, WORD attr, BOOL update = FALSE)
{
	DWORD result = 0;
	static CHAR_INFO* buf = NULL; // 因非多线程，不存在重入问题
	COORD bufsize = { size.X, size.Y }; // 缓冲区大小
	COORD coord = { 0, 0 }; // 起绘点
	//SMALL_RECT rect = { 0, 0, size.X - 1, size.Y - 1 }; // 绘制区域，控制绘制的矩形范围
	SMALL_RECT rect = g_small_rect; // 绘制区域，控制绘制的矩形范围

	if (!ostd)
		return result;

	if (!buf)
	{
		buf = new CHAR_INFO[size.X * size.Y]; // 缓冲区映射buf
		ReadConsoleOutput(ostd, buf, bufsize, coord, &rect);
	}
	for (int i = 0; i < len; i++)
	{
		int bufi = size.X * pt[i].Y + pt[i].X;
		if (bufi >= 0 && bufi < size.X * size.Y
			&& pt[i].X >= 0 && pt[i].X < size.X)
		{
			buf[bufi].Char.UnicodeChar = TEXT(' ');
			if (buf[bufi].Attributes & (BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED))// 发生碰撞，简易思路，当该点绘制的格子不是黑色时表示发生碰撞
				result = 1;
			buf[bufi].Attributes = attr;
		}
	}

	if (update)
		WriteConsoleOutput(ostd, buf, bufsize, coord, &rect);

	return result;
}
/// <summary>
/// 滚动点集，pt[0]的被舍弃，新点添加到点集尾，其他依次向前滚动
/// </summary>
/// <param name="new_pt">新点</param>
/// <param name="pt">点集</param>
/// <param name="len">点集大小</param>
/// <returns>被抛弃的点</returns>
COORD ScrollPT(COORD new_pt, COORD* pt, int len)
{
	COORD old = pt[0];
	for (int i = 0; i < len - 1; i++)
	{
		pt[i] = pt[i + 1];
	}
	pt[len - 1] = new_pt;

	return old;
}
/// <summary>
/// 处理game 的 方向 碰撞 绘图等
/// </summary>
/// <param name="ostd"></param>
/// <param name="size">缓冲区大小</param>
/// <param name="vkcode">按键码</param>
void GameProcess(HANDLE ostd, COORD size, WORD vkcode)
{
	COORD pt = g_newpt;
	COORD old;

	switch (vkcode)
	{
	case VK_LEFT:
		pt.X -= 1;
		break;
	case VK_RIGHT:
		pt.X += 1;
		break;
	case VK_UP:
		pt.Y -= 1;
		break;
	case VK_DOWN:
		pt.Y += 1;
		break;
	default:
		return; // 直接退出。
	}

	if (pt.X == g_boxpt.X && pt.Y == g_boxpt.Y) // 蛇头碰到box，创建新box
	{
		ExpandPT(pt, &g_snake, &g_snake_len);//增大蛇身
		// 更新下一个盒子
		g_boxpt.X = 1 + rand() % (size.X - 2);
		g_boxpt.Y = 1 + rand() % (size.Y - 2);
		ConsoleDraw(ostd, size, &pt, 1, g_ground_attr);//清除box，避免出现碰撞检测
		g_box_score++;
		g_snake_speed = 1 + 0.1 * g_box_score;//慢慢加速
		ShowInfo();
	}
	else
	{
		old = ScrollPT(pt, g_snake, g_snake_len);
		ConsoleDraw(ostd, size, &old, 1, g_ground_attr);//清除尾巴
	}

	ConsoleDraw(ostd, size, g_snake, g_snake_len - 1, g_move_attr);//画身体
	if (1 == ConsoleDraw(ostd, size, &pt, 1, g_head_attr))
		g_run = FALSE;// 发生碰撞，停止game 运行
	ConsoleDraw(ostd, size, &g_boxpt, 1, g_box_attr, TRUE);// 画box

	// 更新蛇头
	g_newpt = pt;
}
/// <summary>
/// input消息处理函数，鼠标的无响应，待解决
/// </summary>
/// <param name="ker"></param>
/// <param name="hConsole"></param>
VOID KeyEventProc(KEY_EVENT_RECORD ker, HANDLE ostd)
{
	if (ker.bKeyDown)
	{
		//GameProcess(ostd, g_size, ker.wVirtualKeyCode);
		switch (ker.wVirtualKeyCode)
		{
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			g_vkcode = ker.wVirtualKeyCode;
			GameProcess(g_ostd, g_size, g_vkcode);
			lasttime = GetTickCount64();
			break;
		default:
			break; // 直接退出。
		}
	}
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
	printf("MouseEventProc\n");

	switch (mer.dwEventFlags)
	{
	case 0:

		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
		}
		else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
		{
		}
		else
		{
		}
		break;
	case DOUBLE_CLICK:
		break;
	case MOUSE_HWHEELED:
		break;
	case MOUSE_MOVED:
		break;
	case MOUSE_WHEELED:
		break;
	default:
		break;
	}
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
	printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}

/// <summary>
/// 初始化game
/// </summary>
void GameInit(void)
{
	DWORD old_mode;

	if (!GetConsoleMode(g_istd, &old_mode))
		return;

	if (!SetConsoleMode(g_istd, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
		return;

	// 隐藏光标
	GetConsoleCursorInfo(g_ostd, &g_cursor_info);
	g_cursor_info.bVisible = FALSE;
	SetConsoleCursorInfo(g_ostd, &g_cursor_info);

	if (!GetConsoleScreenBufferInfo(g_ostd, &g_csbi))
	{
		return;
	}

	// 初始化数据
	g_snake_len = 5; // 出生5节
	g_snake = new COORD[g_snake_len];
	g_vkcode = VK_RIGHT; // 出生往右
	for (int i = 0; i < g_snake_len; i++)
	{
		g_snake[i].X = i + 1;
		g_snake[i].Y = 1;
	}


	// 限制区域
	g_small_rect = { (SHORT)(g_csbi.dwSize.X / 2) , 0,(SHORT)(g_csbi.dwSize.X - 1), (SHORT)(g_csbi.dwSize.Y - 1 - 5) };
	g_size = { (SHORT)(g_small_rect.Right - g_small_rect.Left + 1), (SHORT)(g_small_rect.Bottom - g_small_rect.Top + 1) };
	g_newpt = g_snake[4];
	// 更新初始位置
	if (-1 == g_boxpt.X && -1 == g_boxpt.Y)
	{
		// 更新下一个盒子
		g_boxpt.X = 1 + rand() % (g_size.X - 2);
		g_boxpt.Y = 1 + rand() % (g_size.Y - 2);
	}
	// 墙壁
	COORD* wall_pt = new COORD[2 * (g_size.X + g_size.Y)];
	for (int i = 0; i < g_size.X; i++)
	{
		wall_pt[i].X = i;
		wall_pt[i].Y = 0;
		wall_pt[i + g_size.X].X = i;
		wall_pt[i + g_size.X].Y = g_size.Y - 1;
	}
	for (int i = 0; i < g_size.Y; i++)
	{
		wall_pt[i + 2 * g_size.X].X = 0;
		wall_pt[i + 2 * g_size.X].Y = i;
		wall_pt[i + 2 * g_size.X + g_size.Y].X = g_size.X - 1;
		wall_pt[i + 2 * g_size.X + g_size.Y].Y = i;
	}
	ConsoleDraw(g_ostd, g_size, wall_pt, 2 * (g_size.X + g_size.Y), g_wall_attr);//画墙壁
	delete[] wall_pt;

	ConsoleDraw(g_ostd, g_size, g_snake, g_snake_len - 1, g_move_attr);//画身体
	ConsoleDraw(g_ostd, g_size, &g_newpt, 1, g_head_attr);// 画头
	ConsoleDraw(g_ostd, g_size, &g_boxpt, 1, g_box_attr, TRUE);// 画box
}
/// <summary>
/// 运行game
/// </summary>
/// <returns>1 run 0 stop or err</returns>
DWORD GameRun(void)
{
	if (g_run)
	{
		DWORD nread;
		INPUT_RECORD* ibuf = NULL;
		// 判读是否有输入事件

		if (!GetNumberOfConsoleInputEvents(g_istd, &nread))
		{
			printf("GetNumberOfConsoleInputEvents error\n");
			return 0;
		}

		if (!nread)
		{
			// 当没有按键时，正常行走
			if (GetTickCount64() - lasttime > 1000 / g_snake_speed)
			{
				lasttime = GetTickCount64();
				GameProcess(g_ostd, g_size, g_vkcode);
			}
			return 1;
		}

		nread = 1;
		ibuf = new INPUT_RECORD[nread];
		// 读取输入
		if (!ReadConsoleInput(
			g_istd,      // input buffer handle
			ibuf,     // buffer to read into
			nread,         // size of read buffer
			&nread)) // number of records read
		{
			printf("ReadConsoleInput error\n");
			return 0;
		}

		// 判断事件类型
		for (int i = 0; i < nread; i++)
		{
			switch (ibuf[i].EventType)
			{
			case KEY_EVENT: // keyboard input
				KeyEventProc(ibuf[i].Event.KeyEvent, g_ostd);
				break;

			case MOUSE_EVENT: // mouse input
				MouseEventProc(ibuf[i].Event.MouseEvent);
				break;

			case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
				ResizeEventProc(ibuf[i].Event.WindowBufferSizeEvent);
				break;

			case FOCUS_EVENT:  // disregard focus events

			case MENU_EVENT:   // disregard menu events
				break;

			default:
				;
				break;
			}
		}
		delete[] ibuf;



		return 1;
	}
	else
	{
		return 0;
	}
}
/// <summary>
/// 游戏结束
/// </summary>
/// <param name=""></param>
void GameOver(void)
{
	if (g_run)
		return;

	TCHAR str[20] = TEXT("GAME OVER!!!");
	DWORD written;
	COORD pos = { (g_small_rect.Left + g_small_rect.Right) / 2,(g_small_rect.Bottom + g_small_rect.Top) / 2 };

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	// Get the number of character cells in the current buffer.
	if (!GetConsoleScreenBufferInfo(g_ostd, &csbi))
	{
		return;
	}

	SetConsoleCursorPosition(g_ostd, pos);
	WriteConsole(g_ostd, str, lstrlen(str), &written, NULL);

	SetConsoleCursorPosition(g_ostd, csbi.dwCursorPosition);

	// 显示光标
	g_cursor_info.bVisible = TRUE;
	SetConsoleCursorInfo(g_ostd, &g_cursor_info);
}

int main()
{
	GameInit();
	while (1)
	{
		if (!GameRun())
		{
			GameOver();
			getchar();
			exit(0);
		}
		Sleep(10);
	}

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
