#define _CRT_SECURE_NO_WARNINGS 1

//-------------------------头文件----------------------------------
#include <stdio.h>
#include <graphics.h>//easyx图形库的头文件,需要安装easyx图形库
#include <time.h>
#include <math.h>
#include "tools.h"//解决图片周围黑边问题
#include "vector2.h"//一些数学的东西
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#define WIN_WIDTH 900//窗口长
#define WIN_HELGHT 600//窗口宽
#define ZM_MAX 10//僵尸个数
//-----------------------------------------------------------------

//---------------------枚举----------------------------------
enum { WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT };//植物种类和数量
enum {GOING,WIN,FAIL};//三种游戏状态
int killCount;//已经杀掉僵尸的个数
int zmCount;//已经出现的僵尸个数
int gameStatus;//当前游戏状态
//-----------------------------------------------------------

//---------------设置图片------------------
IMAGE imgBg;//关卡背景
IMAGE imgBar;//背包
IMAGE imgCards[ZHI_WU_COUNT];//植物卡牌
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//植物
IMAGE imgZmStand[11];//转场的僵尸
IMAGE imghand[9];//僵尸手
//-----------------------------------------

//-------------------全局变量---------------------------------
int curX, curY;//当前选中的植物，在移动过程中的位置
int curZhiWu = 0;//选择哪种植物 0:没有选中 1:选择了第一种植物
int sunshine;//定义阳光值
//------------------------------------------------------------

//-----------------植物属性----------------------------------------------------------------
struct zhiwu
{
	int type;//0:没有植物	1:选择了第一种植物
	int frameIndex;//序列帧的序号
	bool catched;//是否被僵尸捕获
	int deadTime;//死亡计数器
	int timer;//定时器
	int x, y;//植物的x，y坐标
	int shootTime;//发射时间
};
struct zhiwu map[3][9];//植物种植的地方
enum {SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};//阳光球的四种状态
//-----------------------------------------------------------------------------------------

//---------------阳光球属性-------------------------
struct sunshineBall
{
	int x, y;//阳光球在飘落过程种的坐标位置(x不变)
	int frameIndex;//当前显示的图片帧的序号
	int destY;//飘落的目标位置的y坐标
	bool used;//是否在使用
	int timer;//计时器
	float xoff;//x偏移量
	float yoff;//y偏移量
	float t;//贝塞尔曲线的时间点
	vector2 p1, p2, p3, p4;//中间四个点
	vector2 pCur;//当前时刻阳光球的位置
	float speed;//速度
	int status;//当前状态
};
struct sunshineBall balls[10];//阳光池
IMAGE imgSunshineBall[29];//阳光球图片数组
//--------------------------------------------------

//-----------------僵尸属性------------------
struct zm
{
	int x, y;//坐标
	int frameIndex;//序列帧的序号
	bool used;//是否登场
	int speed;//速度
	int row;//行
	int blood;//血量
	int dead;//死亡
	bool eating;//正在吃植物
};
struct zm zms[10];//僵尸数量
IMAGE imgZM[21];//图片数组
IMAGE imgZMDead[20];//死亡图片数组
IMAGE imgZMEat[21];//吃东西图片数组
//-------------------------------------------

//----------------子弹的数据类型------------------
struct bullet
{
	int x, y;//坐标
	int row;//行
	bool used;//是否使用
	int speed;//速度
	bool blast;//是否发射爆炸
	int frameIndex;//帧序号
};
struct bullet bullets[30];//子弹池
IMAGE imgBulletNormal;//正常状态下子弹的图片
IMAGE imgBullBlast[4];//图片帧数组
//------------------------------------------------

//---------判断文件是否存在---------
bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}
	else
	{
		fclose(fp);
		return true;
	}
}
//----------------------------------

//--------------------------游戏初始化--------------------------------------------------
void gameInit()//游戏初始化
{
	//加载图片
	loadimage(&imgBg, "res/bg.jpg");//加载关卡背景图片
	loadimage(&imgBar, "res/bar5.png");//加载工具栏图片
	//初始化
	memset(imgZhiWu, 0, sizeof(imgZhiWu));//植物所有元素设置为0
	memset(map, 0, sizeof(map));//植物种植的地方都为0
	killCount = 0;//杀掉的僵尸数量
	zmCount = 0;//已经出现的僵尸
	gameStatus = GOING;//游戏进行
	//初始化植物卡牌
	char name[64];//存储
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		//生成植物卡牌的文件名
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);//加载植物卡片
		loadimage(&imgCards[i], name);//加载植物卡片
		//加载植物
		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);//加载植物
			//先判断这个文件是否存在
			if (fileExist(name))
			{
				imgZhiWu[i][j] = new IMAGE;//分配内存(c++)
				loadimage(imgZhiWu[i][j], name);//加载植物
			}
			else
			{
				break;
			}
		}
	}
	curZhiWu = 0;//没有选中植物
	sunshine = 50;//初始化阳光值
	memset(balls, 0, sizeof(balls));//阳光球初始化
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);//加载阳光球
		loadimage(&imgSunshineBall[i], name);//加载阳光球
	}
	//配置随机种子
	srand(time(NULL));
	//创建游戏的图形窗口
	initgraph(WIN_WIDTH, WIN_HELGHT);
	//设置字体
	LOGFONT f;//定义一个字体文件
	gettextstyle(&f);//获取当前的字体
	f.lfHeight = 30;//字体高度30
	f.lfWeight = 15;//字体宽度15；
	strcpy(f.lfFaceName, "Segoe UI Black");//字体名字
	f.lfQuality = ANTIALIASED_QUALITY;//抗锯齿
	settextstyle(&f);//设置字体
	setbkmode(TRANSPARENT);//字体背景设置透明
	setcolor(BLACK);//输出文本颜色
	//初始化僵尸数据
	memset(zms, 0, sizeof(zms));//初始化僵尸
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);//加载僵尸
		loadimage(&imgZM[i], name);//加载僵尸
	}
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");//加载子弹图片
	memset(bullets, 0, sizeof(bullets));//初始化子弹池
	//初始化豌豆子弹的帧图片数组
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;//加载子弹变小
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png", imgBullBlast[3].getwidth() * k, imgBullBlast[3].getheight() * k, true);//子弹宽度，子弹宽度，等比例放大和缩小
	}
	//初始化僵尸死亡的帧图片数组
	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);//加载图片
		loadimage(&imgZMDead[i], name);//加载图片
	}
	//初始化僵尸吃东西的帧图片数组
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);//加载图片
		loadimage(&imgZMEat[i],name);//加载图片
	}
	//初始化转场的僵尸
	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);//加载图片
		loadimage(&imgZmStand[i], name);//加载图片
	}
	//初始化僵尸手
	for (int i = 0; i < 9; i++)
	{
		sprintf_s(name, "res/hand/%d.png", i + 1);
		loadimage(&imghand[i], name);
	}
}
//---------------------------------------------------------------------------------------

//---------------打印僵尸----------------------------------------------------------------
void drawZM()//打印僵尸
{
	int zmCount = sizeof(zms) / sizeof(zms[0]);//僵尸池的僵尸数
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used)//使用
		{
			IMAGE* img = NULL;//指针
			if (zms[i].dead) img = imgZMDead;//指向死亡
			else if (zms[i].eating) img = imgZMEat;//指向吃东西
			else img = imgZM;//指向行走
			img += zms[i].frameIndex;//帧动作
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);//渲染从左上角开始
		}
	}
}
//---------------------------------------------------------------------------------------

//-----------------------绘制阳光------------------------------------------
void drawSunshines()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);//判断阳光池个数
	for (int i = 0; i < ballMax; i++)
	{
		if(balls[i].used)
		{
			//pCur贝塞尔曲线
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];//阳光帧
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);//打印阳光球
		}
	}
}
//---------------------------------------------------------------------------

//------------------更新窗口--------------------------------------------------------------------
void updateWindow()//更新窗口
{
	BeginBatchDraw();//开始缓冲
	putimage(-112,0,&imgBg);//打印关卡背景图片
	putimagePNG(250, 0, &imgBar);//打印背包图片
	//打印植物卡片
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 338 + i * 65;//植物卡片x坐标
		int y = 6;//植物卡片y坐标
		putimage(x, y, &imgCards[i]);//打印植物卡片
	}
	//打印种植的植物
	for (int i = 0; i < 3; i++)//第几行
	{
		for (int j = 0; j < 9; j++)//第几列
		{ 
			if (map[i][j].type > 0)//种植植物
			{
				int zhiWuType = map[i][j].type - 1;//判断第几种植物
				int index = map[i][j].frameIndex;//植物序列帧
				putimagePNG(map[i][j].x,map[i][j].y,imgZhiWu[zhiWuType][index]);//打印植物
			}
		}
	}
	//打印拖动过程中的植物
	if (curZhiWu == 1 && sunshine >= 100)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];//判断哪种植物
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);//打印植物
	}
	else if (curZhiWu == 2 && sunshine >= 50)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];//判断哪种植物
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);//打印植物
	}
	//打印阳光球
	drawSunshines();//绘制阳光
	char scoreText[8];//分数文本
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);//指定格式的字符串打印到数组
	outtextxy(276,67,scoreText);//在指定位置输出文本
	drawZM();//打印僵尸
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//子弹数
	for (int i = 0; i < bulletMax; i++)
	{
		if (bullets[i].used)
		{
			if (bullets[i].blast)//子弹是否爆炸
			{
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];//播放爆炸动画
				putimagePNG(bullets[i].x, bullets[i].y, img);//打印爆炸子弹
			}
			else
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);//打印子弹
			}
		}
	}
	EndBatchDraw();//结束双缓冲
}
//----------------------------------------------------------------------------------------------

//-------------------------收集阳光-----------------------------------------------------------------
void collectSunshine(ExMessage* msg)//收集阳光
{
	int count = sizeof(balls) / sizeof(balls[0]);//计算阳光池数量
	int w = imgSunshineBall[0].getwidth();//阳光球的宽
	int h = imgSunshineBall[0].getheight();//阳光球的高
	for (int i = 0; i < count; i++)
	{
		if (balls[i].used)//在使用
		{
			int x = balls[i].pCur.x;//x
			int y = balls[i].pCur.y;//y
			if (msg->x > x && msg->x<x + w && msg->y>y && msg->y < y + h)//判断是否点击到阳光球
			{
				balls[i].status = SUNSHINE_COLLECT;//收集状态
				mciSendString("play res/sunshine.mp3", 0, 0, 0);//播放收集阳光的声音
				//设置阳光球的偏移量
				balls[i].p1 = balls[i].pCur;//起点
				balls[i].p4 = vector2(262, 0);//终点
				balls[i].t = 0;//时间点
				float distance = dis(balls[i].p1 - balls[i].p4);//两点间距离 c++重载
				float off = 8;//每次移动8像素
				balls[i].speed = 1.0 / (distance / off);//总距离/每次移动的距离=移动多少次
				break;
			}
		}
	}
}
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------用户操作------------------------------------------------------
void userClick()//用户操作
{
	ExMessage msg;//存放消息
	static int status = 0;//判断鼠标是否拖动
	if (peekmessage(&msg))//判断当前有没有消息
	{
		if (msg.message == WM_LBUTTONDOWN)//鼠标左键按下
		{
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96)//判断鼠标是否点击植物卡片
			{
				int index = (msg.x - 335) / 65;//判断鼠标点击哪个植物卡片
				if (index == 0 && sunshine >= 100)
				{
					mciSendString("play res/卡片槽种子升起.wav", 0, 0, 0);
				}
				else if(index == 1 && sunshine >= 50)
				{
					mciSendString("play res/卡片槽种子升起.wav", 0, 0, 0);
				}
				else
				{
					mciSendString("play res/阳光不足.wav", 0, 0, 0);
				}
				status = 1;//鼠标拖动
				curZhiWu = index + 1;//选中的植物
			}
			else//收集阳光
			{
				collectSunshine(&msg);//收集阳光
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)//鼠标移动
		{
				curX = msg.x;//移动过程中x的坐标
				curY = msg.y;//移动过程中y的坐标
		}
		else if (msg.message == WM_LBUTTONUP && status == 1)//鼠标左键抬起
		{
			if (msg.x > 256-112 && msg.y > 179 && msg.y < 489)//种植范围(只能种植草坪)
			{
				int row = (msg.y - 179) / 102;//判断第几行
				int col = (msg.x - (256 - 112)) / 81;//判断第几列
				if (map[row][col].type == 0)//判断这个地方没有种植植物
				{
					if (curZhiWu == 1 && sunshine >= 100)
					{
						sunshine -= 100;
						mciSendString("play res/zhongzhi.wav", 0, 0, 0);
						map[row][col].type = curZhiWu;//种植植物
						map[row][col].frameIndex = 0;//植物序列帧初始化
						map[row][col].shootTime = 0;
						map[row][col].x = 256 - 112 + col * 81;//x坐标
						map[row][col].y = 179 + row * 102 + 14;//y坐标
					}
					else if (curZhiWu == 2 && sunshine >= 50)
					{
						sunshine -= 50;
						mciSendString("play res/zhongzhi.wav", 0, 0, 0);
						map[row][col].type = curZhiWu;//种植植物
						map[row][col].frameIndex = 0;//植物序列帧初始化
						map[row][col].shootTime = 0;
						map[row][col].x = 256 - 112 + col * 81;//x坐标
						map[row][col].y = 179 + row * 102 + 14;//y坐标
					}
				}
			}
			curZhiWu = 0;//没有选中植物
			status = 0;//鼠标没有拖动
		}
	}
}
//----------------------------------------------------------------------------------------------------------------

//------------------创建阳光------------------------------------------------------------------------------------------------------------------------------------------
void createSunshine()//创建阳光
{
	static int count = 0;//计时器
	static int fre = 500;//频率
	count++;
	if (count >= fre)//400帧率
	{
		fre = 750 + rand() % 250;
		count = 0;//计时器清零
		//从阳光池中取一个可以使用的
		int ballMax = sizeof(balls) / sizeof(balls[0]);//计算阳光池大小
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);//判断是否在使用
		if (i >= ballMax)return;//阳光生产满了
		balls[i].used = true;//使用
		balls[i].frameIndex = 0;//阳光球的图片帧
		balls[i].timer = 0;//定时器设置为0
		balls[i].status = SUNSHINE_DOWN;//阳光状态落下
		balls[i].t = 0;//时间
		balls[i].p1 = vector2(260-112+rand()%(900-(260-112)),60);//起点
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);//终点
		int off = 2;//移动的像素
		float distance = balls[i].p4.y - balls[i].p1.y;//总距离
		balls[i].speed = 1.0 / (distance / off);//移动多少次
	}
	//向日葵生产阳光
	int ballMax = sizeof(balls) / sizeof(balls[0]);//阳光数量
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == XIANG_RI_KUI+1)//如果植物是向日葵
			{
				map[i][j].timer++;//计时器
				if (map[i][j].timer > 750)//原来200
				{
					map[i][j].timer = 0;//重置计时器
					//从阳光池取阳光
					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);//判断是否被使用
					if (k >= ballMax)return;//无可用阳光
					balls[k].used = true;//设置被使用
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);//起点
					int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);//阳光球在左边或者右边100~149距离产出
					balls[k].p4 = vector2(map[i][j].x + w, map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() - imgSunshineBall[0].getheight());//终点
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCT;//生产阳光的状态
					balls[k].speed = 0.05;//速度
					balls[k].t = 0;//时间
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//--------------------更新阳光--------------------------------------------------------------------
void updateSunshine()//更新阳光
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);//判断阳光池个数
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)//判断是否被使用
		{
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;//帧
			if (balls[i].status == SUNSHINE_DOWN)//阳光落下
			{
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND)//阳光点击
			{
				balls[i].timer++;
				if (balls[i].timer > 200)//原100
				{
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT)//阳光收集
			{
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1)
				{
					sun->used = false;
					sunshine += 25;
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT)//阳光生产
			{
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------------------

//-------创建僵尸---------------------------------------------------------
void createZM()//创建僵尸
{
	if (zmCount >= ZM_MAX)
	{
		return;
	}
	static int count = 0;//计数器
	static int zmFre = 500;//间隔500创建僵尸
	count++;
	if (count >= zmFre)
	{
		zmFre = rand() % 200 + 300;//300~500
		count = 0;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);//僵尸池数量
		for (i = 0; i < zmMax && zms[i].used; i++);//已经使用
		if (i < zmMax)
		{
			memset(&zms[i], 0, sizeof(zms[i]));//全部设置0
			zms[i].used = true;//使用
			zms[i].x = WIN_WIDTH;//x坐标为窗口宽度
			zms[i].row = rand() % 3;//0~2
			zms[i].y = 172 + (1 + zms[i].row) * 100;//随机数1-3其中一行
			zms[1].speed = 1;//速度为1
			zms[i].blood = 200;//僵尸血量初始化100
			zms[i].dead = false;//重置add
			zmCount++;
		}
	}
}
//------------------------------------------------------------------------

//--------------更新僵尸的状态---------------
void updateZM()//更新僵尸状态
{
	static int count = 0;//计时器
	count++;
	int zmMax = sizeof(zms) / sizeof(zms[0]);//僵尸池数量
	if (count > 2*2)
	{
		count = 0;
		//更新僵尸的位置
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)//在使用
			{
				zms[i].x -= zms[i].speed;//僵尸移动
				if (i == 1 && zms[i].x ==850)
				{
					mciSendString("play res/僵尸警报.mp3", 0, 0, 0);
				}
				if (zms[i].x < 48)//到达房子
				{
					gameStatus = FAIL;
				}
			}
		}
	}
	static int count2 = 0;//计时器
	count2++;
	if (count2 > 4*2)
	{
		count2 = 0;
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)//在使用
			{
				if (zms[i].dead)//僵尸死亡
				{
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20)
					{
						zms[i].used = false;//僵尸消失
						mciSendString("play res/僵尸倒下1.mp3", 0, 0, 0);
						killCount++;
						if (killCount == ZM_MAX)
						{
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating)
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;//帧动作
					mciSendString("play res/啃食声1.wav", 0, 0, 0);
				}
				else
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;//帧动作
				}
			}
		}
	}
}
//-------------------------------------------

//-----发射豌豆子弹------------------------------------------------------------------------------------------
void shoot()
{
	static int count = 0;
	if (++count < 5)return;
	count = 0;
	int lines[3] = { 0 };//三行
	int zmCount = sizeof(zms) / sizeof(zms[0]);//僵尸个数
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//子弹个数
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();//危险距离
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used && zms[i].x < dangerX)//判断僵尸是否存在，是否子弹的射程
		{
			lines[zms[i].row] = 1;//当前行有僵尸
		}
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == WAN_DOU + 1&&lines[i])//植物是否是豌豆,当前行是否有僵尸
			{
				map[i][j].shootTime++;//计时器
				if(map[i][j].shootTime > 20)//大于20帧发射
				{
					map[i][j].shootTime = 0;//重置
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);//子弹为使用并且子弹池还有子弹
					if (k < bulletMax)//子弹池还有子弹
					{
						bullets[k].used = true;//使用
						bullets[k].row = i;//行
						bullets[k].speed = 6;//速度
						bullets[k].blast = false;//子弹刚开始没有爆炸
						bullets[k].frameIndex = 0;//帧序号为0
						int zwX = 256-112 + j * 81;//种植植物x坐标
						int zwY = 179 + i * 102 + 14;//种植植物y坐标
						bullets[k].x = zwX + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;//子弹x坐标
						bullets[k].y = zwY + 5;//子弹y坐标
					}
				}
			}
		}
	}
}
//------------------------------------------------------------------------------------------------------------

//-----------------更新豌豆子弹---------------------------------------
void updateBullets()//更新豌豆子弹
{
	static int count = 0;
	if (++count < 2)return;
	count = 0;
	int countMax = sizeof(bullets) / sizeof(bullets[0]);//子弹数量
	for (int i = 0; i < countMax; i++)
	{
		if (bullets[i].used)//使用
		{
			bullets[i].x += bullets[i].speed;//子弹移动
			if (bullets[i].x > WIN_WIDTH)//碰到边缘
			{
				bullets[i].used = false;//回收子弹
			}
			if (bullets[i].blast)//子弹爆炸
			{
				bullets[i].frameIndex++;//播放帧图片
				if (bullets[i].frameIndex >= 4)//播放结束
				{
					bullets[i].used = false;//子弹消失
				}
			}
		}
	}
}
//--------------------------------------------------------------------

//------------------------检查子弹对僵尸的影响-----------------------------------------------------------------------------------
void checkBulletZZm()
{
	int bCount = sizeof(bullets) / sizeof(bullets[0]);//子弹数量
	int zCount = sizeof(zms) / sizeof(zms[0]);//僵尸数量
	for (int i = 0; i < bCount; i++)
	{
		if (bullets[i].used == false || bullets[i].blast)continue;//子弹没有发射和子弹已经爆炸
		for (int k = 0; k < zCount; k++)
		{
			if (zms[k].used == false)continue;//僵尸没有出来
			{
				int x1 = zms[k].x + 80;//僵尸的范围
				int x2 = zms[k].x + 110;//僵尸的范围
				int x = bullets[i].x;//子弹x坐标
				if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2)// 子弹和僵尸在同一行并且碰到僵尸
				{
					mciSendString("play res/豌豆击中僵尸.wav", 0, 0, 0);
					zms[k].blood -= 10;//僵尸扣血
					bullets[i].blast = true;//爆炸
					bullets[i].speed = 0;//速度停止
					if (zms[k].blood <= 0)//没血了
					{
						zms[k].speed = 0;//速度为停止
						zms[k].dead = true;//死亡状态
						zms[k].frameIndex = 0;//图片帧为0
					}
					break;
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------

//------------------僵尸对植物的检测----------------------------------------
void checkZm2ZhiWU()
{
	int zCount = sizeof(zms) / sizeof(zms[0]);//僵尸数量
	for (int i = 0; i < zCount; i++)
	{
		if (zms[i].dead)continue;//僵尸死亡
		int row = zms[i].row;//僵尸的这行
		for (int k = 0; k < 9; k++)//一行9个植物
		{
			if (map[row][k].type == 0)continue;//没有植物
			int zhiWuX = 256-112 + k * 81;//植物x坐标
			int x1 = zhiWuX + 10;//植物左边界
			int x2 = zhiWuX + 60;//植物右边界
			int x3 = zms[i].x + 80;//僵尸的左边界
			if (x3 > x1 && x3 < x2)
			{
				if (map[row][k].catched)//几行几列的植物给抓了
				{
					map[row][k].deadTime++;//计时
					if (map[row][k].deadTime > 100)
					{
						map[row][k].deadTime = 0;//死亡计时器重置
						map[row][k].type = 0;//植物消失
						zms[i].eating = false;//不吃了
						zms[i].frameIndex = 0;//帧为0
						zms[i].speed = 1;//继续走
					}
				}
				else
				{
					map[row][k].catched = true;//被抓住
					map[row][k].deadTime = 0;//死亡计时器设置0
					zms[i].eating = true;//正在吃
					zms[i].speed = 0;//速度为0
					zms[i].frameIndex = 0;//帧初始化
				}
			}
		}
	}
}
//--------------------------------------------------------------------------

//----------碰撞检测--------------------------------------------------------------------------
void collisionCheck()
{
	checkBulletZZm();//子弹对僵尸的碰撞检测
	checkZm2ZhiWU();//僵尸对植物的检测
}
//---------------------------------------------------------------------------------------------

//-------------------------------------更新植物------------------------------
void updateZhiWu()
{
	static int count = 0;
	if (++count < 6)return;//帧率
	count = 0;
	for (int i = 0; i < 3; i++)//第几行
	{
		for (int j = 0; j < 9; j++)//第几列
		{
			if (map[i][j].type > 0)//存在植物
			{
				map[i][j].frameIndex++;//改变帧(植物动作)
				int zhiWuType = map[i][j].type - 1;//植物类型
				int index = map[i][j].frameIndex;//植物帧
				if (imgZhiWu[zhiWuType][index] == NULL)//植物帧为最后一张
				{
					map[i][j].frameIndex = 0;//植物帧为第一张
				}
			}
		}
	}
}
//---------------------------------------------------------------------------

//--------------游戏更新------------------------------------------------------------
void updateGame()//游戏更新
{
	//植物动作更新
	updateZhiWu();
	createSunshine();//创建阳光
	updateSunshine();//更新阳光
	createZM();//创建僵尸
	updateZM();//更新僵尸的状态
	shoot();//发送豌豆子弹
	updateBullets();//更新豌豆子弹
	collisionCheck();//实现豌豆子弹和僵尸碰撞检测
}
//------------------------------------------------------------------------------------

//--------------启动菜单-------------------------------------------------------------------------------------------------------------------------------
void startUI()//启动菜单
{
	mciSendString("play res/menu.mp3 repeat", 0, 0, 0);//循环播放
	IMAGE imgBg, imgMenu1, imgMenu2, imgWoodSign1, imgWoodSign2, imgWoodSign3;//启动菜单
	loadimage(&imgBg, "res/menu.png");//加载
	loadimage(&imgMenu1, "res/menu1.png");//加载
	loadimage(&imgMenu2, "res/menu2.png");//加载
	loadimage(&imgWoodSign1, "res/WoodSign1.png");//加载
	loadimage(&imgWoodSign2, "res/WoodSign2.png");//加载
	loadimage(&imgWoodSign3, "res/WoodSign3.png");//加载
	int flag = 0;//判断鼠标是否按下
	putimage(0, 0, &imgBg);//打印背景
	//木栏下滑
	mciSendString("play res/木栏.mp3", 0, 0, 0);
	int height1 = imgWoodSign1.getheight();
	for (int y = -height1; y < 0; y++)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//打印背景
		putimagePNG(60, y, &imgWoodSign1);//木栏1
		putimagePNG(474, 75, &imgMenu1);//打印按钮
		EndBatchDraw();
	}
	int height2 = imgWoodSign2.getheight();
	for (int y = -height2; y < 0; y++)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//打印背景
		putimagePNG(60, 0, &imgWoodSign1);//木栏1
		putimagePNG(60, y, &imgWoodSign2);//木栏2
		putimagePNG(474, 75, &imgMenu1);//打印按钮
		EndBatchDraw();
	}
	int height3 = imgWoodSign3.getheight();
	for (int y = -height3; y < 165; y++)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//打印背景
		putimagePNG(60, 0, &imgWoodSign1);//木栏1
		putimagePNG(60, 115, &imgWoodSign2);//木栏2
		putimagePNG(60, y, &imgWoodSign3);//木栏3
		putimagePNG(474, 75, &imgMenu1);//打印按钮
		EndBatchDraw();
	}
	while (1)
	{
		BeginBatchDraw();//缓冲开始
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);//打印按钮
		ExMessage msg;//存储消息
		if (peekmessage(&msg))//判断是否有消息
		{
			if (msg.message == WM_LBUTTONDOWN && msg.x > 474 && msg.x < 474 + 300 && msg.y>75 && msg.y < 75 + 140)//按下鼠标左键并判断是否点击按钮
			{
				flag = 1;//鼠标按下
			}
			else if (msg.message == WM_LBUTTONUP && flag)//鼠标左键抬起
			{
				mciSendString("play res/僵尸狂笑声.mp3", 0, 0, 0);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(350, 500, &imghand[0]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(340, 325, &imghand[1]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(300, 280, &imghand[2]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(280, 250, &imghand[3]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(235, 250, &imghand[4]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(235, 250, &imghand[5]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(270, 230, &imghand[6]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(270, 230, &imghand[7]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//打印背景
				putimagePNG(60, 0, &imgWoodSign1);//木栏1
				putimagePNG(60, 115, &imgWoodSign2);//木栏2
				putimagePNG(60, 165, &imgWoodSign3);//木栏3
				putimagePNG(474, 75, &imgMenu1);//打印按钮
				putimagePNG(260, 240, &imghand[8]);
				EndBatchDraw();
				Sleep(2000);
				break;
				EndBatchDraw();//结束缓冲
			}
		}
		EndBatchDraw();//结束缓冲
	}
}
//---------------------------------------------------------------------------------------------------------------------------------------------------

//----------转场----------------------------------------
void viewScence()
{
	mciSendString("stop res/menu.mp3", 0, 0, 0);//停止播放
	mciSendString("play res/zhuanchang.wav", 0, 0, 0);//循环播放
	int xMin = WIN_WIDTH - imgBg.getwidth();//900-1400=-500
	vector2 points[9]{ {550,80},{530,160},{630,170},{530,200},{515,270},{565,370},{605,340},{705,280},{690,340} };//僵尸对应的左边
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;//随机的帧
	}
	int count = 0;
	for (int x = 0; x >= xMin; x -= 2)
	{
		BeginBatchDraw();//开始缓冲
		putimage(x, 0, &imgBg);//转场
		count++;
		//绘制僵尸
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZmStand[index[k]]);//绘制僵尸
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;//切换帧
			}
		}
		if (count >= 10)count = 0;//重置
		EndBatchDraw();//结束缓冲
		Sleep(5);//帧等待
	}
	//停留1S左右
	for (int i = 0; i < 100; i++)
	{
		BeginBatchDraw();
		putimage(xMin, 0, &imgBg);
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x, points[k].y, &imgZmStand[index[k]]);
			index[k] = (index[k] + 1) % 11;
		}
		EndBatchDraw();
		Sleep(50);
	}
	//往回转场
	for (int x = xMin; x <= -112; x += 2)
	{
		BeginBatchDraw();
		putimage(x, 0, &imgBg);
		count++;
		for (int k = 0; k < 9; k++)
		{
			(points[k].x - xMin + x, points[k].y, &imgZmStand[index[k]]);
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;
			}
			if (count >= 10)count = 0;//重置
		}
		EndBatchDraw();
		Sleep(5);
	}
	mciSendString("stop res/zhuanchang.wav", 0, 0, 0);//循环播放
	mciSendString("play res/Grasswalk.mp3 repeat", 0, 0, 0);//循环播放
}
//------------------------------------------------------

//----------工具栏下滑---------------------------
void barsDown()
{
	int height = imgBar.getheight();
	for (int y = -height; y <= 0; y++)
	{
		BeginBatchDraw();
		putimage(-112, 0, & imgBg);
		putimage(250, y, &imgBar);
		for (int i = 0; i < ZHI_WU_COUNT; i++)
		{
			int x = 338 + i * 65;
			putimage(x, 6+y, &imgCards[i]);
		}
		EndBatchDraw();
		Sleep(10);
	}
}
//-----------------------------------------------

//------------------判断游戏结束-----------------
bool checkOver()
{
	int ret = false;
	if (gameStatus == WIN)
	{
		Sleep(2000);
		loadimage(0, "res/win2.png");
		mciSendString("stop res/Grasswalk.mp3", 0, 0, 0);//停止播放
		mciSendString("play res/win.mp3", 0, 0, 0);//播放播放
		ret = true;
	}
	else if (gameStatus == FAIL)
	{
		Sleep(2000);
		loadimage(0, "res/fail2.png");
		mciSendString("stop res/Grasswalk.mp3", 0, 0, 0);//停止播放
		mciSendString("play res/lose.mp3", 0, 0, 0);//播放播放
		ret = true;
	}
	return ret;
}
//------------------------------------------------

//-------------主函数------------------------------
int main(void)
{
	gameInit();//游戏初始化
	startUI();//启动菜单
	viewScence();//转场
	barsDown();//工具栏
	int timer = 0;//计时器
	bool flag = true;//判断帧更新
	while (1)
	{
		userClick();//用户操作
		timer += getDelay();//自定义记录时间间隔
		if (timer > 10)//时间间隔大于20
		{
			flag = true;//为真
			timer = 0;//计算器为0
		}
		if (flag)//判断
		{
			flag = false;//为假
			updateWindow();//更新窗口
			updateGame();//游戏更新
			if (checkOver())break;
		}
	}
	system("pause");//窗口停留
	return 0;
}
//---------------------------------------------------