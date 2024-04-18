#define _CRT_SECURE_NO_WARNINGS 1

//-------------------------ͷ�ļ�----------------------------------
#include <stdio.h>
#include <graphics.h>//easyxͼ�ο��ͷ�ļ�,��Ҫ��װeasyxͼ�ο�
#include <time.h>
#include <math.h>
#include "tools.h"//���ͼƬ��Χ�ڱ�����
#include "vector2.h"//һЩ��ѧ�Ķ���
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#define WIN_WIDTH 900//���ڳ�
#define WIN_HELGHT 600//���ڿ�
#define ZM_MAX 10//��ʬ����
//-----------------------------------------------------------------

//---------------------ö��----------------------------------
enum { WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT };//ֲ�����������
enum {GOING,WIN,FAIL};//������Ϸ״̬
int killCount;//�Ѿ�ɱ����ʬ�ĸ���
int zmCount;//�Ѿ����ֵĽ�ʬ����
int gameStatus;//��ǰ��Ϸ״̬
//-----------------------------------------------------------

//---------------����ͼƬ------------------
IMAGE imgBg;//�ؿ�����
IMAGE imgBar;//����
IMAGE imgCards[ZHI_WU_COUNT];//ֲ�￨��
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//ֲ��
IMAGE imgZmStand[11];//ת���Ľ�ʬ
IMAGE imghand[9];//��ʬ��
//-----------------------------------------

//-------------------ȫ�ֱ���---------------------------------
int curX, curY;//��ǰѡ�е�ֲ����ƶ������е�λ��
int curZhiWu = 0;//ѡ������ֲ�� 0:û��ѡ�� 1:ѡ���˵�һ��ֲ��
int sunshine;//��������ֵ
//------------------------------------------------------------

//-----------------ֲ������----------------------------------------------------------------
struct zhiwu
{
	int type;//0:û��ֲ��	1:ѡ���˵�һ��ֲ��
	int frameIndex;//����֡�����
	bool catched;//�Ƿ񱻽�ʬ����
	int deadTime;//����������
	int timer;//��ʱ��
	int x, y;//ֲ���x��y����
	int shootTime;//����ʱ��
};
struct zhiwu map[3][9];//ֲ����ֲ�ĵط�
enum {SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};//�����������״̬
//-----------------------------------------------------------------------------------------

//---------------����������-------------------------
struct sunshineBall
{
	int x, y;//��������Ʈ������ֵ�����λ��(x����)
	int frameIndex;//��ǰ��ʾ��ͼƬ֡�����
	int destY;//Ʈ���Ŀ��λ�õ�y����
	bool used;//�Ƿ���ʹ��
	int timer;//��ʱ��
	float xoff;//xƫ����
	float yoff;//yƫ����
	float t;//���������ߵ�ʱ���
	vector2 p1, p2, p3, p4;//�м��ĸ���
	vector2 pCur;//��ǰʱ���������λ��
	float speed;//�ٶ�
	int status;//��ǰ״̬
};
struct sunshineBall balls[10];//�����
IMAGE imgSunshineBall[29];//������ͼƬ����
//--------------------------------------------------

//-----------------��ʬ����------------------
struct zm
{
	int x, y;//����
	int frameIndex;//����֡�����
	bool used;//�Ƿ�ǳ�
	int speed;//�ٶ�
	int row;//��
	int blood;//Ѫ��
	int dead;//����
	bool eating;//���ڳ�ֲ��
};
struct zm zms[10];//��ʬ����
IMAGE imgZM[21];//ͼƬ����
IMAGE imgZMDead[20];//����ͼƬ����
IMAGE imgZMEat[21];//�Զ���ͼƬ����
//-------------------------------------------

//----------------�ӵ�����������------------------
struct bullet
{
	int x, y;//����
	int row;//��
	bool used;//�Ƿ�ʹ��
	int speed;//�ٶ�
	bool blast;//�Ƿ��䱬ը
	int frameIndex;//֡���
};
struct bullet bullets[30];//�ӵ���
IMAGE imgBulletNormal;//����״̬���ӵ���ͼƬ
IMAGE imgBullBlast[4];//ͼƬ֡����
//------------------------------------------------

//---------�ж��ļ��Ƿ����---------
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

//--------------------------��Ϸ��ʼ��--------------------------------------------------
void gameInit()//��Ϸ��ʼ��
{
	//����ͼƬ
	loadimage(&imgBg, "res/bg.jpg");//���عؿ�����ͼƬ
	loadimage(&imgBar, "res/bar5.png");//���ع�����ͼƬ
	//��ʼ��
	memset(imgZhiWu, 0, sizeof(imgZhiWu));//ֲ������Ԫ������Ϊ0
	memset(map, 0, sizeof(map));//ֲ����ֲ�ĵط���Ϊ0
	killCount = 0;//ɱ���Ľ�ʬ����
	zmCount = 0;//�Ѿ����ֵĽ�ʬ
	gameStatus = GOING;//��Ϸ����
	//��ʼ��ֲ�￨��
	char name[64];//�洢
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		//����ֲ�￨�Ƶ��ļ���
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);//����ֲ�￨Ƭ
		loadimage(&imgCards[i], name);//����ֲ�￨Ƭ
		//����ֲ��
		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);//����ֲ��
			//���ж�����ļ��Ƿ����
			if (fileExist(name))
			{
				imgZhiWu[i][j] = new IMAGE;//�����ڴ�(c++)
				loadimage(imgZhiWu[i][j], name);//����ֲ��
			}
			else
			{
				break;
			}
		}
	}
	curZhiWu = 0;//û��ѡ��ֲ��
	sunshine = 50;//��ʼ������ֵ
	memset(balls, 0, sizeof(balls));//�������ʼ��
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);//����������
		loadimage(&imgSunshineBall[i], name);//����������
	}
	//�����������
	srand(time(NULL));
	//������Ϸ��ͼ�δ���
	initgraph(WIN_WIDTH, WIN_HELGHT);
	//��������
	LOGFONT f;//����һ�������ļ�
	gettextstyle(&f);//��ȡ��ǰ������
	f.lfHeight = 30;//����߶�30
	f.lfWeight = 15;//������15��
	strcpy(f.lfFaceName, "Segoe UI Black");//��������
	f.lfQuality = ANTIALIASED_QUALITY;//�����
	settextstyle(&f);//��������
	setbkmode(TRANSPARENT);//���屳������͸��
	setcolor(BLACK);//����ı���ɫ
	//��ʼ����ʬ����
	memset(zms, 0, sizeof(zms));//��ʼ����ʬ
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);//���ؽ�ʬ
		loadimage(&imgZM[i], name);//���ؽ�ʬ
	}
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");//�����ӵ�ͼƬ
	memset(bullets, 0, sizeof(bullets));//��ʼ���ӵ���
	//��ʼ���㶹�ӵ���֡ͼƬ����
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;//�����ӵ���С
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png", imgBullBlast[3].getwidth() * k, imgBullBlast[3].getheight() * k, true);//�ӵ���ȣ��ӵ���ȣ��ȱ����Ŵ����С
	}
	//��ʼ����ʬ������֡ͼƬ����
	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);//����ͼƬ
		loadimage(&imgZMDead[i], name);//����ͼƬ
	}
	//��ʼ����ʬ�Զ�����֡ͼƬ����
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);//����ͼƬ
		loadimage(&imgZMEat[i],name);//����ͼƬ
	}
	//��ʼ��ת���Ľ�ʬ
	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);//����ͼƬ
		loadimage(&imgZmStand[i], name);//����ͼƬ
	}
	//��ʼ����ʬ��
	for (int i = 0; i < 9; i++)
	{
		sprintf_s(name, "res/hand/%d.png", i + 1);
		loadimage(&imghand[i], name);
	}
}
//---------------------------------------------------------------------------------------

//---------------��ӡ��ʬ----------------------------------------------------------------
void drawZM()//��ӡ��ʬ
{
	int zmCount = sizeof(zms) / sizeof(zms[0]);//��ʬ�صĽ�ʬ��
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used)//ʹ��
		{
			IMAGE* img = NULL;//ָ��
			if (zms[i].dead) img = imgZMDead;//ָ������
			else if (zms[i].eating) img = imgZMEat;//ָ��Զ���
			else img = imgZM;//ָ������
			img += zms[i].frameIndex;//֡����
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);//��Ⱦ�����Ͻǿ�ʼ
		}
	}
}
//---------------------------------------------------------------------------------------

//-----------------------��������------------------------------------------
void drawSunshines()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);//�ж�����ظ���
	for (int i = 0; i < ballMax; i++)
	{
		if(balls[i].used)
		{
			//pCur����������
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];//����֡
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);//��ӡ������
		}
	}
}
//---------------------------------------------------------------------------

//------------------���´���--------------------------------------------------------------------
void updateWindow()//���´���
{
	BeginBatchDraw();//��ʼ����
	putimage(-112,0,&imgBg);//��ӡ�ؿ�����ͼƬ
	putimagePNG(250, 0, &imgBar);//��ӡ����ͼƬ
	//��ӡֲ�￨Ƭ
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 338 + i * 65;//ֲ�￨Ƭx����
		int y = 6;//ֲ�￨Ƭy����
		putimage(x, y, &imgCards[i]);//��ӡֲ�￨Ƭ
	}
	//��ӡ��ֲ��ֲ��
	for (int i = 0; i < 3; i++)//�ڼ���
	{
		for (int j = 0; j < 9; j++)//�ڼ���
		{ 
			if (map[i][j].type > 0)//��ֲֲ��
			{
				int zhiWuType = map[i][j].type - 1;//�жϵڼ���ֲ��
				int index = map[i][j].frameIndex;//ֲ������֡
				putimagePNG(map[i][j].x,map[i][j].y,imgZhiWu[zhiWuType][index]);//��ӡֲ��
			}
		}
	}
	//��ӡ�϶������е�ֲ��
	if (curZhiWu == 1 && sunshine >= 100)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];//�ж�����ֲ��
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);//��ӡֲ��
	}
	else if (curZhiWu == 2 && sunshine >= 50)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];//�ж�����ֲ��
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);//��ӡֲ��
	}
	//��ӡ������
	drawSunshines();//��������
	char scoreText[8];//�����ı�
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);//ָ����ʽ���ַ�����ӡ������
	outtextxy(276,67,scoreText);//��ָ��λ������ı�
	drawZM();//��ӡ��ʬ
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//�ӵ���
	for (int i = 0; i < bulletMax; i++)
	{
		if (bullets[i].used)
		{
			if (bullets[i].blast)//�ӵ��Ƿ�ը
			{
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];//���ű�ը����
				putimagePNG(bullets[i].x, bullets[i].y, img);//��ӡ��ը�ӵ�
			}
			else
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);//��ӡ�ӵ�
			}
		}
	}
	EndBatchDraw();//����˫����
}
//----------------------------------------------------------------------------------------------

//-------------------------�ռ�����-----------------------------------------------------------------
void collectSunshine(ExMessage* msg)//�ռ�����
{
	int count = sizeof(balls) / sizeof(balls[0]);//�������������
	int w = imgSunshineBall[0].getwidth();//������Ŀ�
	int h = imgSunshineBall[0].getheight();//������ĸ�
	for (int i = 0; i < count; i++)
	{
		if (balls[i].used)//��ʹ��
		{
			int x = balls[i].pCur.x;//x
			int y = balls[i].pCur.y;//y
			if (msg->x > x && msg->x<x + w && msg->y>y && msg->y < y + h)//�ж��Ƿ�����������
			{
				balls[i].status = SUNSHINE_COLLECT;//�ռ�״̬
				mciSendString("play res/sunshine.mp3", 0, 0, 0);//�����ռ����������
				//�����������ƫ����
				balls[i].p1 = balls[i].pCur;//���
				balls[i].p4 = vector2(262, 0);//�յ�
				balls[i].t = 0;//ʱ���
				float distance = dis(balls[i].p1 - balls[i].p4);//�������� c++����
				float off = 8;//ÿ���ƶ�8����
				balls[i].speed = 1.0 / (distance / off);//�ܾ���/ÿ���ƶ��ľ���=�ƶ����ٴ�
				break;
			}
		}
	}
}
//----------------------------------------------------------------------------------------------------

//--------------------------------------------------�û�����------------------------------------------------------
void userClick()//�û�����
{
	ExMessage msg;//�����Ϣ
	static int status = 0;//�ж�����Ƿ��϶�
	if (peekmessage(&msg))//�жϵ�ǰ��û����Ϣ
	{
		if (msg.message == WM_LBUTTONDOWN)//����������
		{
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96)//�ж�����Ƿ���ֲ�￨Ƭ
			{
				int index = (msg.x - 335) / 65;//�ж�������ĸ�ֲ�￨Ƭ
				if (index == 0 && sunshine >= 100)
				{
					mciSendString("play res/��Ƭ����������.wav", 0, 0, 0);
				}
				else if(index == 1 && sunshine >= 50)
				{
					mciSendString("play res/��Ƭ����������.wav", 0, 0, 0);
				}
				else
				{
					mciSendString("play res/���ⲻ��.wav", 0, 0, 0);
				}
				status = 1;//����϶�
				curZhiWu = index + 1;//ѡ�е�ֲ��
			}
			else//�ռ�����
			{
				collectSunshine(&msg);//�ռ�����
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)//����ƶ�
		{
				curX = msg.x;//�ƶ�������x������
				curY = msg.y;//�ƶ�������y������
		}
		else if (msg.message == WM_LBUTTONUP && status == 1)//������̧��
		{
			if (msg.x > 256-112 && msg.y > 179 && msg.y < 489)//��ֲ��Χ(ֻ����ֲ��ƺ)
			{
				int row = (msg.y - 179) / 102;//�жϵڼ���
				int col = (msg.x - (256 - 112)) / 81;//�жϵڼ���
				if (map[row][col].type == 0)//�ж�����ط�û����ֲֲ��
				{
					if (curZhiWu == 1 && sunshine >= 100)
					{
						sunshine -= 100;
						mciSendString("play res/zhongzhi.wav", 0, 0, 0);
						map[row][col].type = curZhiWu;//��ֲֲ��
						map[row][col].frameIndex = 0;//ֲ������֡��ʼ��
						map[row][col].shootTime = 0;
						map[row][col].x = 256 - 112 + col * 81;//x����
						map[row][col].y = 179 + row * 102 + 14;//y����
					}
					else if (curZhiWu == 2 && sunshine >= 50)
					{
						sunshine -= 50;
						mciSendString("play res/zhongzhi.wav", 0, 0, 0);
						map[row][col].type = curZhiWu;//��ֲֲ��
						map[row][col].frameIndex = 0;//ֲ������֡��ʼ��
						map[row][col].shootTime = 0;
						map[row][col].x = 256 - 112 + col * 81;//x����
						map[row][col].y = 179 + row * 102 + 14;//y����
					}
				}
			}
			curZhiWu = 0;//û��ѡ��ֲ��
			status = 0;//���û���϶�
		}
	}
}
//----------------------------------------------------------------------------------------------------------------

//------------------��������------------------------------------------------------------------------------------------------------------------------------------------
void createSunshine()//��������
{
	static int count = 0;//��ʱ��
	static int fre = 500;//Ƶ��
	count++;
	if (count >= fre)//400֡��
	{
		fre = 750 + rand() % 250;
		count = 0;//��ʱ������
		//���������ȡһ������ʹ�õ�
		int ballMax = sizeof(balls) / sizeof(balls[0]);//��������ش�С
		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);//�ж��Ƿ���ʹ��
		if (i >= ballMax)return;//������������
		balls[i].used = true;//ʹ��
		balls[i].frameIndex = 0;//�������ͼƬ֡
		balls[i].timer = 0;//��ʱ������Ϊ0
		balls[i].status = SUNSHINE_DOWN;//����״̬����
		balls[i].t = 0;//ʱ��
		balls[i].p1 = vector2(260-112+rand()%(900-(260-112)),60);//���
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);//�յ�
		int off = 2;//�ƶ�������
		float distance = balls[i].p4.y - balls[i].p1.y;//�ܾ���
		balls[i].speed = 1.0 / (distance / off);//�ƶ����ٴ�
	}
	//���տ���������
	int ballMax = sizeof(balls) / sizeof(balls[0]);//��������
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == XIANG_RI_KUI+1)//���ֲ�������տ�
			{
				map[i][j].timer++;//��ʱ��
				if (map[i][j].timer > 750)//ԭ��200
				{
					map[i][j].timer = 0;//���ü�ʱ��
					//�������ȡ����
					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);//�ж��Ƿ�ʹ��
					if (k >= ballMax)return;//�޿�������
					balls[k].used = true;//���ñ�ʹ��
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);//���
					int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);//����������߻����ұ�100~149�������
					balls[k].p4 = vector2(map[i][j].x + w, map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() - imgSunshineBall[0].getheight());//�յ�
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCT;//���������״̬
					balls[k].speed = 0.05;//�ٶ�
					balls[k].t = 0;//ʱ��
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//--------------------��������--------------------------------------------------------------------
void updateSunshine()//��������
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);//�ж�����ظ���
	for (int i = 0; i < ballMax; i++)
	{
		if (balls[i].used)//�ж��Ƿ�ʹ��
		{
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;//֡
			if (balls[i].status == SUNSHINE_DOWN)//��������
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
			else if (balls[i].status == SUNSHINE_GROUND)//������
			{
				balls[i].timer++;
				if (balls[i].timer > 200)//ԭ100
				{
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT)//�����ռ�
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
			else if (balls[i].status == SUNSHINE_PRODUCT)//��������
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

//-------������ʬ---------------------------------------------------------
void createZM()//������ʬ
{
	if (zmCount >= ZM_MAX)
	{
		return;
	}
	static int count = 0;//������
	static int zmFre = 500;//���500������ʬ
	count++;
	if (count >= zmFre)
	{
		zmFre = rand() % 200 + 300;//300~500
		count = 0;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);//��ʬ������
		for (i = 0; i < zmMax && zms[i].used; i++);//�Ѿ�ʹ��
		if (i < zmMax)
		{
			memset(&zms[i], 0, sizeof(zms[i]));//ȫ������0
			zms[i].used = true;//ʹ��
			zms[i].x = WIN_WIDTH;//x����Ϊ���ڿ��
			zms[i].row = rand() % 3;//0~2
			zms[i].y = 172 + (1 + zms[i].row) * 100;//�����1-3����һ��
			zms[1].speed = 1;//�ٶ�Ϊ1
			zms[i].blood = 200;//��ʬѪ����ʼ��100
			zms[i].dead = false;//����add
			zmCount++;
		}
	}
}
//------------------------------------------------------------------------

//--------------���½�ʬ��״̬---------------
void updateZM()//���½�ʬ״̬
{
	static int count = 0;//��ʱ��
	count++;
	int zmMax = sizeof(zms) / sizeof(zms[0]);//��ʬ������
	if (count > 2*2)
	{
		count = 0;
		//���½�ʬ��λ��
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)//��ʹ��
			{
				zms[i].x -= zms[i].speed;//��ʬ�ƶ�
				if (i == 1 && zms[i].x ==850)
				{
					mciSendString("play res/��ʬ����.mp3", 0, 0, 0);
				}
				if (zms[i].x < 48)//���﷿��
				{
					gameStatus = FAIL;
				}
			}
		}
	}
	static int count2 = 0;//��ʱ��
	count2++;
	if (count2 > 4*2)
	{
		count2 = 0;
		for (int i = 0; i < zmMax; i++)
		{
			if (zms[i].used)//��ʹ��
			{
				if (zms[i].dead)//��ʬ����
				{
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20)
					{
						zms[i].used = false;//��ʬ��ʧ
						mciSendString("play res/��ʬ����1.mp3", 0, 0, 0);
						killCount++;
						if (killCount == ZM_MAX)
						{
							gameStatus = WIN;
						}
					}
				}
				else if (zms[i].eating)
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;//֡����
					mciSendString("play res/��ʳ��1.wav", 0, 0, 0);
				}
				else
				{
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;//֡����
				}
			}
		}
	}
}
//-------------------------------------------

//-----�����㶹�ӵ�------------------------------------------------------------------------------------------
void shoot()
{
	static int count = 0;
	if (++count < 5)return;
	count = 0;
	int lines[3] = { 0 };//����
	int zmCount = sizeof(zms) / sizeof(zms[0]);//��ʬ����
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);//�ӵ�����
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();//Σ�վ���
	for (int i = 0; i < zmCount; i++)
	{
		if (zms[i].used && zms[i].x < dangerX)//�жϽ�ʬ�Ƿ���ڣ��Ƿ��ӵ������
		{
			lines[zms[i].row] = 1;//��ǰ���н�ʬ
		}
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type == WAN_DOU + 1&&lines[i])//ֲ���Ƿ����㶹,��ǰ���Ƿ��н�ʬ
			{
				map[i][j].shootTime++;//��ʱ��
				if(map[i][j].shootTime > 20)//����20֡����
				{
					map[i][j].shootTime = 0;//����
					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);//�ӵ�Ϊʹ�ò����ӵ��ػ����ӵ�
					if (k < bulletMax)//�ӵ��ػ����ӵ�
					{
						bullets[k].used = true;//ʹ��
						bullets[k].row = i;//��
						bullets[k].speed = 6;//�ٶ�
						bullets[k].blast = false;//�ӵ��տ�ʼû�б�ը
						bullets[k].frameIndex = 0;//֡���Ϊ0
						int zwX = 256-112 + j * 81;//��ֲֲ��x����
						int zwY = 179 + i * 102 + 14;//��ֲֲ��y����
						bullets[k].x = zwX + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;//�ӵ�x����
						bullets[k].y = zwY + 5;//�ӵ�y����
					}
				}
			}
		}
	}
}
//------------------------------------------------------------------------------------------------------------

//-----------------�����㶹�ӵ�---------------------------------------
void updateBullets()//�����㶹�ӵ�
{
	static int count = 0;
	if (++count < 2)return;
	count = 0;
	int countMax = sizeof(bullets) / sizeof(bullets[0]);//�ӵ�����
	for (int i = 0; i < countMax; i++)
	{
		if (bullets[i].used)//ʹ��
		{
			bullets[i].x += bullets[i].speed;//�ӵ��ƶ�
			if (bullets[i].x > WIN_WIDTH)//������Ե
			{
				bullets[i].used = false;//�����ӵ�
			}
			if (bullets[i].blast)//�ӵ���ը
			{
				bullets[i].frameIndex++;//����֡ͼƬ
				if (bullets[i].frameIndex >= 4)//���Ž���
				{
					bullets[i].used = false;//�ӵ���ʧ
				}
			}
		}
	}
}
//--------------------------------------------------------------------

//------------------------����ӵ��Խ�ʬ��Ӱ��-----------------------------------------------------------------------------------
void checkBulletZZm()
{
	int bCount = sizeof(bullets) / sizeof(bullets[0]);//�ӵ�����
	int zCount = sizeof(zms) / sizeof(zms[0]);//��ʬ����
	for (int i = 0; i < bCount; i++)
	{
		if (bullets[i].used == false || bullets[i].blast)continue;//�ӵ�û�з�����ӵ��Ѿ���ը
		for (int k = 0; k < zCount; k++)
		{
			if (zms[k].used == false)continue;//��ʬû�г���
			{
				int x1 = zms[k].x + 80;//��ʬ�ķ�Χ
				int x2 = zms[k].x + 110;//��ʬ�ķ�Χ
				int x = bullets[i].x;//�ӵ�x����
				if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2)// �ӵ��ͽ�ʬ��ͬһ�в���������ʬ
				{
					mciSendString("play res/�㶹���н�ʬ.wav", 0, 0, 0);
					zms[k].blood -= 10;//��ʬ��Ѫ
					bullets[i].blast = true;//��ը
					bullets[i].speed = 0;//�ٶ�ֹͣ
					if (zms[k].blood <= 0)//ûѪ��
					{
						zms[k].speed = 0;//�ٶ�Ϊֹͣ
						zms[k].dead = true;//����״̬
						zms[k].frameIndex = 0;//ͼƬ֡Ϊ0
					}
					break;
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------

//------------------��ʬ��ֲ��ļ��----------------------------------------
void checkZm2ZhiWU()
{
	int zCount = sizeof(zms) / sizeof(zms[0]);//��ʬ����
	for (int i = 0; i < zCount; i++)
	{
		if (zms[i].dead)continue;//��ʬ����
		int row = zms[i].row;//��ʬ������
		for (int k = 0; k < 9; k++)//һ��9��ֲ��
		{
			if (map[row][k].type == 0)continue;//û��ֲ��
			int zhiWuX = 256-112 + k * 81;//ֲ��x����
			int x1 = zhiWuX + 10;//ֲ����߽�
			int x2 = zhiWuX + 60;//ֲ���ұ߽�
			int x3 = zms[i].x + 80;//��ʬ����߽�
			if (x3 > x1 && x3 < x2)
			{
				if (map[row][k].catched)//���м��е�ֲ���ץ��
				{
					map[row][k].deadTime++;//��ʱ
					if (map[row][k].deadTime > 100)
					{
						map[row][k].deadTime = 0;//������ʱ������
						map[row][k].type = 0;//ֲ����ʧ
						zms[i].eating = false;//������
						zms[i].frameIndex = 0;//֡Ϊ0
						zms[i].speed = 1;//������
					}
				}
				else
				{
					map[row][k].catched = true;//��ץס
					map[row][k].deadTime = 0;//������ʱ������0
					zms[i].eating = true;//���ڳ�
					zms[i].speed = 0;//�ٶ�Ϊ0
					zms[i].frameIndex = 0;//֡��ʼ��
				}
			}
		}
	}
}
//--------------------------------------------------------------------------

//----------��ײ���--------------------------------------------------------------------------
void collisionCheck()
{
	checkBulletZZm();//�ӵ��Խ�ʬ����ײ���
	checkZm2ZhiWU();//��ʬ��ֲ��ļ��
}
//---------------------------------------------------------------------------------------------

//-------------------------------------����ֲ��------------------------------
void updateZhiWu()
{
	static int count = 0;
	if (++count < 6)return;//֡��
	count = 0;
	for (int i = 0; i < 3; i++)//�ڼ���
	{
		for (int j = 0; j < 9; j++)//�ڼ���
		{
			if (map[i][j].type > 0)//����ֲ��
			{
				map[i][j].frameIndex++;//�ı�֡(ֲ�ﶯ��)
				int zhiWuType = map[i][j].type - 1;//ֲ������
				int index = map[i][j].frameIndex;//ֲ��֡
				if (imgZhiWu[zhiWuType][index] == NULL)//ֲ��֡Ϊ���һ��
				{
					map[i][j].frameIndex = 0;//ֲ��֡Ϊ��һ��
				}
			}
		}
	}
}
//---------------------------------------------------------------------------

//--------------��Ϸ����------------------------------------------------------------
void updateGame()//��Ϸ����
{
	//ֲ�ﶯ������
	updateZhiWu();
	createSunshine();//��������
	updateSunshine();//��������
	createZM();//������ʬ
	updateZM();//���½�ʬ��״̬
	shoot();//�����㶹�ӵ�
	updateBullets();//�����㶹�ӵ�
	collisionCheck();//ʵ���㶹�ӵ��ͽ�ʬ��ײ���
}
//------------------------------------------------------------------------------------

//--------------�����˵�-------------------------------------------------------------------------------------------------------------------------------
void startUI()//�����˵�
{
	mciSendString("play res/menu.mp3 repeat", 0, 0, 0);//ѭ������
	IMAGE imgBg, imgMenu1, imgMenu2, imgWoodSign1, imgWoodSign2, imgWoodSign3;//�����˵�
	loadimage(&imgBg, "res/menu.png");//����
	loadimage(&imgMenu1, "res/menu1.png");//����
	loadimage(&imgMenu2, "res/menu2.png");//����
	loadimage(&imgWoodSign1, "res/WoodSign1.png");//����
	loadimage(&imgWoodSign2, "res/WoodSign2.png");//����
	loadimage(&imgWoodSign3, "res/WoodSign3.png");//����
	int flag = 0;//�ж�����Ƿ���
	putimage(0, 0, &imgBg);//��ӡ����
	//ľ���»�
	mciSendString("play res/ľ��.mp3", 0, 0, 0);
	int height1 = imgWoodSign1.getheight();
	for (int y = -height1; y < 0; y++)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//��ӡ����
		putimagePNG(60, y, &imgWoodSign1);//ľ��1
		putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
		EndBatchDraw();
	}
	int height2 = imgWoodSign2.getheight();
	for (int y = -height2; y < 0; y++)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//��ӡ����
		putimagePNG(60, 0, &imgWoodSign1);//ľ��1
		putimagePNG(60, y, &imgWoodSign2);//ľ��2
		putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
		EndBatchDraw();
	}
	int height3 = imgWoodSign3.getheight();
	for (int y = -height3; y < 165; y++)
	{
		BeginBatchDraw();
		putimage(0, 0, &imgBg);//��ӡ����
		putimagePNG(60, 0, &imgWoodSign1);//ľ��1
		putimagePNG(60, 115, &imgWoodSign2);//ľ��2
		putimagePNG(60, y, &imgWoodSign3);//ľ��3
		putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
		EndBatchDraw();
	}
	while (1)
	{
		BeginBatchDraw();//���忪ʼ
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);//��ӡ��ť
		ExMessage msg;//�洢��Ϣ
		if (peekmessage(&msg))//�ж��Ƿ�����Ϣ
		{
			if (msg.message == WM_LBUTTONDOWN && msg.x > 474 && msg.x < 474 + 300 && msg.y>75 && msg.y < 75 + 140)//�������������ж��Ƿ�����ť
			{
				flag = 1;//��갴��
			}
			else if (msg.message == WM_LBUTTONUP && flag)//������̧��
			{
				mciSendString("play res/��ʬ��Ц��.mp3", 0, 0, 0);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(350, 500, &imghand[0]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(340, 325, &imghand[1]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(300, 280, &imghand[2]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(280, 250, &imghand[3]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(235, 250, &imghand[4]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(235, 250, &imghand[5]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(270, 230, &imghand[6]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(270, 230, &imghand[7]);
				EndBatchDraw();
				Sleep(100);
				BeginBatchDraw();
				putimage(0, 0, &imgBg);//��ӡ����
				putimagePNG(60, 0, &imgWoodSign1);//ľ��1
				putimagePNG(60, 115, &imgWoodSign2);//ľ��2
				putimagePNG(60, 165, &imgWoodSign3);//ľ��3
				putimagePNG(474, 75, &imgMenu1);//��ӡ��ť
				putimagePNG(260, 240, &imghand[8]);
				EndBatchDraw();
				Sleep(2000);
				break;
				EndBatchDraw();//��������
			}
		}
		EndBatchDraw();//��������
	}
}
//---------------------------------------------------------------------------------------------------------------------------------------------------

//----------ת��----------------------------------------
void viewScence()
{
	mciSendString("stop res/menu.mp3", 0, 0, 0);//ֹͣ����
	mciSendString("play res/zhuanchang.wav", 0, 0, 0);//ѭ������
	int xMin = WIN_WIDTH - imgBg.getwidth();//900-1400=-500
	vector2 points[9]{ {550,80},{530,160},{630,170},{530,200},{515,270},{565,370},{605,340},{705,280},{690,340} };//��ʬ��Ӧ�����
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;//�����֡
	}
	int count = 0;
	for (int x = 0; x >= xMin; x -= 2)
	{
		BeginBatchDraw();//��ʼ����
		putimage(x, 0, &imgBg);//ת��
		count++;
		//���ƽ�ʬ
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZmStand[index[k]]);//���ƽ�ʬ
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;//�л�֡
			}
		}
		if (count >= 10)count = 0;//����
		EndBatchDraw();//��������
		Sleep(5);//֡�ȴ�
	}
	//ͣ��1S����
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
	//����ת��
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
			if (count >= 10)count = 0;//����
		}
		EndBatchDraw();
		Sleep(5);
	}
	mciSendString("stop res/zhuanchang.wav", 0, 0, 0);//ѭ������
	mciSendString("play res/Grasswalk.mp3 repeat", 0, 0, 0);//ѭ������
}
//------------------------------------------------------

//----------�������»�---------------------------
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

//------------------�ж���Ϸ����-----------------
bool checkOver()
{
	int ret = false;
	if (gameStatus == WIN)
	{
		Sleep(2000);
		loadimage(0, "res/win2.png");
		mciSendString("stop res/Grasswalk.mp3", 0, 0, 0);//ֹͣ����
		mciSendString("play res/win.mp3", 0, 0, 0);//���Ų���
		ret = true;
	}
	else if (gameStatus == FAIL)
	{
		Sleep(2000);
		loadimage(0, "res/fail2.png");
		mciSendString("stop res/Grasswalk.mp3", 0, 0, 0);//ֹͣ����
		mciSendString("play res/lose.mp3", 0, 0, 0);//���Ų���
		ret = true;
	}
	return ret;
}
//------------------------------------------------

//-------------������------------------------------
int main(void)
{
	gameInit();//��Ϸ��ʼ��
	startUI();//�����˵�
	viewScence();//ת��
	barsDown();//������
	int timer = 0;//��ʱ��
	bool flag = true;//�ж�֡����
	while (1)
	{
		userClick();//�û�����
		timer += getDelay();//�Զ����¼ʱ����
		if (timer > 10)//ʱ��������20
		{
			flag = true;//Ϊ��
			timer = 0;//������Ϊ0
		}
		if (flag)//�ж�
		{
			flag = false;//Ϊ��
			updateWindow();//���´���
			updateGame();//��Ϸ����
			if (checkOver())break;
		}
	}
	system("pause");//����ͣ��
	return 0;
}
//---------------------------------------------------