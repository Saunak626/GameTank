#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "resource.h"
#include "Tank.h"
#include "Gdi.h"

#define TankScreenX         (SCREEN_X)//��Ļ����
#define TankScreenY         (SCREEN_Y)//��Ļ���
#define TankPntSize         (MAX_PNT_SIZE)//���ص����ű���

#define TankDbgTextSize     (20)

#define TankPointXY         (TankPntSize)//���ص����ű���
#define TankMaxX            ((TankScreenX) / (TankPointXY))//���ź��x���������ֵ
#define TankMaxY            ((TankScreenY) / (TankPointXY))//���ź��y���������ֵ

#define TANK_SHAPE_NUM_MAX      (4u)//�ܹ�����̹����״����Ӧ���������ĸ�����
#define TANK_SHAPE_PNT_MAX      (6u)//ÿ��̹��������������ʾ

#define TANK_ENMY_WAR_NUM_MAX   (((TankMaxX * TankMaxY) / 6) - 1)//�о���ս��̹���������
#define TANK_ENMY_WAR_NUM_MIN   (5u)//�о���ս��̹����С����(���)
#define TANK_MYSELF_WAR_NUM_MAX (1u)//�Ҿ���ս��̹���������
#define TANK_MYSELF_WAR_NUM_MIN (1u)//�Ҿ���ս��̹����С����(���)
#define TANK_WAR_NUM_MAX        (TANK_ENMY_WAR_NUM_MAX + TANK_MYSELF_WAR_NUM_MAX)//̹���������
#define TANK_BOMB_NUM_MAX       (TANK_WAR_NUM_MAX * 3)//�ڵ��������
#define TANK_ALL_NUM_MIN        (TANK_ENMY_WAR_NUM_MIN + TANK_MYSELF_WAR_NUM_MIN)
#define TANK_ALL_NUM_MAX        (TANK_ENMY_WAR_NUM_MAX + TANK_MYSELF_WAR_NUM_MAX)
#define TANK_ENMY_NUM_INIT      (TANK_ALL_NUM_MIN)//��ʼ̹������

#define TANK_SPEED_MAX          (5u)
#define TANK_TIMER_MIN          (0u)
#define TANK_TIMER_MAX          (100u)

#define TANK_TIMER_FIRE_MIN     (TANK_TIMER_MIN)
#define TANK_TIMER_FIRE_MAX     (TANK_TIMER_MAX)

#define TANK_TIMER_EQUIP_MIN    (TANK_TIMER_MAX)
#define TANK_TIMER_EQUIP_MAX    (TANK_TIMER_MAX*10)

#define TANK_WEAPON_LIFE_MAX    (5u)//ÿ���������ʹ�ô���

#define TANK_OF_MYSELF          (TANK_WAR_BOX[0])

#define COPY_POINT(pdpnt, pspnt)                \
do                                              \
{                                               \
    (pdpnt)->x = (pspnt)->x;                    \
    (pdpnt)->y = (pspnt)->y;                    \
    (pdpnt)->col = (pspnt)->col;                \
}while(0)

#define COPY_TANK(pdtank, pstank)                       \
do                                                      \
{                                                       \
    COPY_POINT(&((pdtank)->pnt), &((pstank)->pnt));     \
    (pdtank)->dir = (pstank)->dir;                      \
    (pdtank)->pr = (pstank)->pr;                        \
    (pdtank)->lf = (pstank)->lf;                        \
    (pdtank)->mv = (pstank)->mv;                        \
}while(0)

typedef enum property_e
{
    PR_MIN = 0,
    PR_NULL = 0,
    PR_WALL, //ǽ
    PR_WEAPON, //����
    PR_LIFE, //װ��
    PR_BOMB, //�ڵ�
    PR_MYSELF, //�Լ�
    PR_ENMY, //����
    PR_TBOMB,
    PR_MAX
} property_t; //����

typedef enum weapon_e
{
    WP_MIN = 0,
    WP_NONE,//��ͨ����
    WP_MAX
}weapon_t;

typedef enum life_e
{
    LF_MIN = 0,
    LF_DIE = 0,//����
    LF_BURN,//ȼ��
    LF_LIVE,//���
    LF_MAX
} life_t;

typedef enum move_e
{
    MV_MIN = 0,
    MV_STOP = 0,//ֹͣ
    MV_MOVE,//�ƶ�
    MV_MAX
}move_t;

typedef struct point_s
{
    uint32_t x;
    uint32_t y;
    uint32_t col;
} point_t;//�����

typedef struct equip_s
{
    int8        valid;
    point_t     pnt;//װ���������
    property_t  pr;//װ������(������������) 
    int32       tmr;//װ����ʱ�������ں�װ����ʧ
} equip_t;

typedef struct tank_s
{
    int8        valid;
    dir_t       dir;//̹�˷���ͬʱҲ������TANK_SHAPE_BOX�е�����
    point_t     pnt;//̹�����Ͻǵ����꣬����������������TANK_SHAPE_BOX��
    property_t  pr;//����
    int32       lf;//����
    int32       wep;//����
    int32       mv;//�ƶ�(����)
    int32       fire;//���𵹼�ʱ
    int32       kill;//��ɱ�ĵо�����
    int32       bomb;//���ĵ�ҩ����
} tank_t;

typedef struct bomb_s
{
    int8        valid;
    dir_t       dir;//�ڵ����з���
    point_t     pnt;//�ڵ��������
    property_t  pr;//�ڵ�����(���˵��ڵ������Լ����ڵ�)
    tank_t*     ptank;//����ڵ�������̹�˷����
} bomb_t;

typedef struct tank_pr_s
{
    int32 cur_tank_num;//��ǰ̹������
    int32 war_tank_num;//��ս��̹������
    //int32 kill_tank_num;//����(ɱ��)̹����
    //int32 cur_bomb_num;//��ǰ�ڵ�����
    int32 create_equp_tmr;//װ��������ʱ�������ں����һ��װ��(����������)
    int32 speed;
    int32 super;//�޵�ģʽ
    int32 debug;//������Ϣ
}tank_pr_t;

typedef struct warmap_s
{
    int32 col;
    int32 pr;
}warmap_t;

const point_t TANK_SHAPE_BOX[TANK_SHAPE_NUM_MAX][TANK_SHAPE_PNT_MAX] =
{//�����ĸ������̹�ˣ��䳵����������е�λ�ö��ǹ̶���
    { { 1, 0, TRUE }, { 0, 1, TRUE }, { 2, 1, TRUE }, { 1, 1, TRUE }, { 0, 2, TRUE }, { 2, 2, TRUE } }, //��
    { { 2, 1, TRUE }, { 1, 0, TRUE }, { 1, 2, TRUE }, { 1, 1, TRUE }, { 0, 0, TRUE }, { 0, 2, TRUE } }, //��
    { { 1, 2, TRUE }, { 2, 1, TRUE }, { 0, 1, TRUE }, { 1, 1, TRUE }, { 2, 0, TRUE }, { 0, 0, TRUE } }, //��
    { { 0, 1, TRUE }, { 1, 2, TRUE }, { 1, 0, TRUE }, { 1, 1, TRUE }, { 2, 2, TRUE }, { 2, 0, TRUE } }  //��
};

const int32 TANK_PR_COLOUR[PR_MAX] =
{
    GDI_RGB_BACKGROUND,//PR_NULL = 0,//�հף���ɫ
    GDI_RGB_FOREGROUND,//PR_WALL, //ǽ����ɫ
    GDI_RGB_PURPLE,//PR_WEAPON, //��������ɫ
    GDI_RGB_GREEN,//PR_LIFE, //װ������ɫ
    GDI_RGB_YELLOW,//PR_BOMB, //�ڵ�����ɫ
    GDI_RGB_RED,//PR_MYSELF, //�Լ�����ɫ
    GDI_RGB_BLUE,//PR_ENMY, //���ˣ���ɫ
    GDI_RGB_YELLOW//PR_TBOMB,//�ջٵ�̹�ˣ���ɫ
};

static tank_t   TANK_WAR_BOX[TANK_WAR_NUM_MAX] = { 0 };//��ս��̹��(���)
//static int32    TANK_WAR_BOX_VALID[TANK_WAR_NUM_MAX] = { 0 };
static bomb_t   TANK_BOMB_BOX[TANK_BOMB_NUM_MAX] = { 0 };//ͬһʱ������̹�˷��������ڵ�
//static int32    TANK_BOMB_BOX_VALID[TANK_BOMB_NUM_MAX] = { 0 };
static warmap_t TANK_WAR_MAP[TankMaxX][TankMaxY] = { 0 };//̹�˴�ս��Ϸ��ͼ,�൱����ʾ����

static equip_t TANK_EQUIP = { 0 };//װ��
static tank_pr_t TANK_PR = { 0 };//����

//static uint32 TANK_DEBUG = FALSE;

void tank_debug_out(void)
{
    int32 i = 0;
    int32 kill = 0, life = 0, enemy = 0, bomb = 0, super = 0;
    TCHAR debugStr[200] = { 0 };
    PTCHAR superStr = NULL;

    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        if ((TANK_WAR_BOX[i].pr != PR_NULL) &&
            (TANK_WAR_BOX[i].valid != FALSE))
        {
            if (TANK_WAR_BOX[i].pr != PR_MYSELF)
            {
                enemy += 1;
            }
            else
            {
                life = TANK_WAR_BOX[i].lf - LF_BURN;
                life = ((life > 0) ? life : 0);
                kill = TANK_WAR_BOX[i].kill;
                bomb = TANK_WAR_BOX[i].bomb;
            }
        }
    }

#ifdef DEBUG_TEXT_OUT_CHN
    superStr = (TANK_PR.super) ? (TEXT("����")) : (TEXT("�ر�"));
    wsprintf(debugStr, TEXT("����ֵ[%03d], ����о�[%03d], �о�����[%03d], ���ĵ�ҩ[%06d], ����ģʽ[%s]"),
        life, kill, enemy, bomb, superStr);
#else
    superStr = (TANK_PR.super) ? (TEXT("ON")) : (TEXT("OFF"));
    wsprintf(debugStr, TEXT("Kill[%03d], Life[%03d], Enemy[%03d], Bomb[%06d], Super[%s]"),
        kill, life, enemy, bomb, superStr);
#endif
    gdi_textout(0, TankScreenY - TankDbgTextSize - 1, TankDbgTextSize, GDI_RGB_FOREGROUND, 0, debugStr);
}

int32 tank_set_super(tank_t* tank, int32 super)
{
    if (super)
    {
        tank->lf = LF_MAX;
        tank->wep = WP_MAX;
    }

    return RTN_OK;
}

//����GDI����һ����״
int32 tank_draw_point(int32 x, int32 y, int32 col, int32 pr)
{
    //pr�������ڿ�����ʾ����״

    //���ص�����
    x = ((x < 0) ? 0 : ((x >= TankMaxX) ? (TankMaxX - 1) : x));
    y = ((y < 0) ? 0 : ((y >= TankMaxY) ? (TankMaxY - 1) : y));

    pr = ((pr >= PR_MAX) ? PR_NULL : ((pr <= PR_NULL) ? PR_NULL : pr));

    if (TankPointXY != 1)//��һ�����δ������Ź���ĵ�
    {
        x = x * TankPointXY;
        y = y * TankPointXY;
        switch (pr)
        {
        case PR_BOMB://�ڵ�����ɫԲ��
            gdi_circle(x + (TankPointXY / 2), y + (TankPointXY / 2), (TankPointXY / 2) - 1, col, 1);
            break;
        case PR_WEAPON://��������ɫ������
            gdi_triangle(x, y, x + TankPointXY - 1, y + TankPointXY - 1, col, 1);
            break;
        case PR_LIFE://װ������ɫ����
            gdi_rhombus(x, y, x + TankPointXY - 1, y + TankPointXY - 1, col, 1);
            break;
        case PR_WALL://ǽ����ɫ����
        case PR_MYSELF://�Ҿ�����ɫ����
        case PR_ENMY://�о�����ɫ����
            gdi_rectangle(x, y, x + TankPointXY - 1, y + TankPointXY - 1, col, 1);
            break;
        case PR_NULL://�հף���ɫ����
        default:
            col = TANK_PR_COLOUR[PR_NULL];
            gdi_rectangle(x, y, x + TankPointXY - 1, y + TankPointXY - 1, col, 1);
            break;
        }
    }
    else//Ϊ�˼ӿ��ٶȣ����Ϊ1�����ţ�ֱ�ӻ���
    {
        gdi_set_point(x, y, col);
    }

    return RTN_OK;
}

//��յ�ͼ�е�����
int32 tank_clear_warmap(void)
{
    int32 x = 0, y = 0;
    for (x = 0; x < TankMaxX; x++)
    {
        for (y = 0; y < TankMaxY; y++)
        {
            TANK_WAR_MAP[x][y].col = TANK_PR_COLOUR[PR_NULL];
            TANK_WAR_MAP[x][y].pr = PR_NULL;
        }
    }

    return RTN_OK;
}

//�ѵ�ͼ���Ƶ���Ļ������
int32 tank_update_warmap(void)
{
    int32 x = 0, y = 0;

    TCHAR debugStr[200] = { 0 };

    gdi_clear(TANK_PR_COLOUR[PR_NULL]);

    for (x = 0; x < TankMaxX; x++)
    {
        for (y = 0; y < TankMaxY; y++)
        {
            tank_draw_point(x, y, TANK_WAR_MAP[x][y].col, TANK_WAR_MAP[x][y].pr);
        }
    }

    if (TANK_PR.debug)
    {
        tank_debug_out();
    }

    gdi_update();
    return RTN_OK;
}

//����ͼ�����һ����״
int32 tank_set_warmap(point_t *point, int32 pr)
{
    //pr�������ڿ�����ʾ����״
    int32 x = 0, y = 0;

    if (point == NULL)
        return RTN_ERR;

    x = point->x;
    y = point->y;

    x = ((x < 0) ? 0 : ((x >= TankMaxX) ? (TankMaxX - 1) : x));
    y = ((y < 0) ? 0 : ((y >= TankMaxY) ? (TankMaxY - 1) : y));

    pr = ((pr >= PR_MAX) ? PR_NULL : ((pr <= PR_NULL) ? PR_NULL : pr));

    //TANK_WAR_MAP[x][y].col = TANK_PR_COLOUR[pr];;
    TANK_WAR_MAP[x][y].col = point->col;
    TANK_WAR_MAP[x][y].pr = pr;

    return RTN_OK;
}

//�ӵ�ͼ�ϻ�ȡһ����״
int32 tank_get_warmap(point_t *point)
{
    int32 x = 0, y = 0;
    int32 col = 0, pr = 0;
    int32 ret = PR_NULL;

    if (point == RTN_NULL)
    {
        return RTN_ERR;
    }

    x = point->x;
    y = point->y;

    x = ((x < 0) ? 0 : ((x >= TankMaxX) ? (TankMaxX - 1) : x));
    y = ((y < 0) ? 0 : ((y >= TankMaxY) ? (TankMaxY - 1) : y));

    return TANK_WAR_MAP[x][y].pr;
}

//rand for [min, max)
int32 tank_get_rand(int32 min, int32 max)
{
    static int32 seed = 0;
    if (seed == 0)
    {
        seed = GetTickCount();
        srand(seed);
    }

    return (min + (rand() % max));
}


//��ȡһ������������
void *tank_get_randpnt(point_t *t_point)
{
    if (t_point == RTN_NULL)
    {
        return RTN_NULL;
    }

    t_point->x = tank_get_rand(0, TankMaxX - 3);
    t_point->y = tank_get_rand(0, TankMaxY - 3);

    return t_point;
}


//���ݷ����ȡ��һ���������
point_t* tank_get_nextpnt(dir_t dir, point_t* t_point)
{
    int32 dx = 0, dy = 0;

    if (t_point == NULL)
    {
        return RTN_NULL;
    }

    dir = (dir <= DIR_MIN) ? DIR_MIN : ((dir >= DIR_MAX) ? (DIR_MAX - 1) : dir);

    switch (dir)
    {
    case DIR_UP:
        dx = 0; dy = -1;
        break;
    case DIR_DOWN:
        dx = 0; dy = 1;
        break;
    case DIR_LEFT:
        dx = -1; dy = 0;
        break;
    case DIR_RIGHT:
        dx = 1; dy = 0;
        break;
    default:
        break;
    }

    t_point->x += dx;
    t_point->y += dy;

    return t_point;
}

//����һ��̹��
tank_t* tank_create_atank(tank_t* tank, int32 pr)
{
    point_t pnt = { 0 };
    int32 n = TankMaxX * TankMaxY;
    int32 i = 0;

    if (tank == RTN_NULL)
    {
        return RTN_NULL;
    }

    memset(tank, 0, sizeof(tank_t));
    tank->valid = TRUE;
    tank->pr = PR_NULL;
    tank->dir = tank_get_rand(DIR_MIN, DIR_MAX);//��ȡһ������ķ���
    tank->wep = WP_NONE;
    tank->lf = LF_LIVE;
    tank->kill = 0;
    tank->bomb = 0;

    if (pr == PR_MYSELF)//�Ҿ�
    {
        tank->mv = MV_STOP;//Ĭ�ϲ����ƶ�,���ֶ��ٿأ�
        tank->fire = 1;
        tank->pr = PR_MYSELF;
    }
    else//�о�
    {
        tank->mv = tank_get_rand(MV_MOVE, min(TankMaxX, TankMaxY));//����һ��������ƶ�����
        tank->fire = tank_get_rand(TANK_TIMER_FIRE_MIN, TANK_TIMER_FIRE_MAX);
        tank->pr = PR_ENMY;
    }

    while (n--)//Ѱ�ҿ��Է���̹�˵������
    {
        tank_get_randpnt(&(tank->pnt));//����һ�������

        for (i = 0; i < TANK_SHAPE_PNT_MAX; i++)
        {
            pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][i].x;
            pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][i].y;
            if (tank_get_warmap(&pnt) != PR_NULL)
            {
                break;
            }
        }

        if (i >= TANK_SHAPE_PNT_MAX)
        {
            tank->pnt.col = TANK_PR_COLOUR[tank->pr];
            return tank;//��λ�ÿ��Է���һ̹��
        }
    }

    memset(tank, 0, sizeof(tank_t));
    tank->pr = PR_NULL;
    tank->valid = FALSE;

    return RTN_NULL;
}

//�ڵ�ͼ�ϻ���̹��
int32 tank_draw_atank(tank_t* tank)
{
    int32 i = 0;
    point_t pnt = { 0 };
    int32 pr = 0;

    if (tank == NULL)
    {
        return RTN_ERR;
    }

    if (tank->valid == FALSE)
    {
        return RTN_ERR;
    }

    for (i = 0; i < TANK_SHAPE_PNT_MAX; i++)
    {
        pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][i].x;
        pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][i].y;
        pnt.col = tank->pnt.col;
        tank_set_warmap(&pnt, tank->pr);
    }

    return RTN_OK;
}

//���̹���Ƿ��ܹ������ƶ�
int32 tank_check_atank(tank_t* tank)
{
    int32 i = 0, pr = 0;
    point_t pnt = { 0 };
    point_t* ppnt = NULL;

    if (tank == NULL)
    {
        return FALSE;
    }

    if ((tank->pr == PR_NULL) || (tank->valid == FALSE))
    {
        return FALSE;
    }

    //̹�˲���Խ��
    if ((tank->pnt.x < 0) || (tank->pnt.x >= TankMaxX) ||
        (tank->pnt.y < 0) || (tank->pnt.y >= TankMaxY))
    {
        return FALSE;
    }

    for (i = 0; i < /*TANK_SHAPE_PNT_MAX*/3; i++)//ֻ��Ҫ���ǰ�������ܷ��ƶ�
    {
        pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][i].x;
        pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][i].y;

        ppnt = tank_get_nextpnt(tank->dir, &pnt);

        if (ppnt == RTN_NULL)
        {
            return FALSE;
        }

        //̹�˲���Խ��
        if ((ppnt->x < 0) || (ppnt->x >= TankMaxX) ||
            (ppnt->y < 0) || (ppnt->y >= TankMaxY))
        {
            return FALSE;
        }

        //̹�˵���һ��λ�ñ��벻��̹�˻�ǽ
        pr = tank_get_warmap(ppnt);
        if ((pr == PR_WALL) || (pr == PR_MYSELF) || (pr == PR_ENMY))
        {
            return FALSE;
        }
    }

    return TRUE;
}

#if 0
//�ڵ�ͼ������̹��
tank_t* tank_search_atank_inmap(point_t* point, tank_t* tank)
{
    int32 i = 0;
    point_t pnt = { 0 };
    int32 pr = 0, dr = 0, j = 0;

    if (point == NULL)
    {
        return RTN_NULL;
    }

    pr = tank_get_warmap(point);

    if ((pr != PR_ENMY) && (pr != PR_MYSELF))
    {
        memset(tank, 0, sizeof(tank_t));
        tank->pr = PR_NULL;
        return RTN_NULL;//����㲻����̹��
    }

    tank->pr = pr;

    //�����������̹����
    for (dr = DIR_MIN; dr < DIR_MAX; dr++)
    {//�ֱ����̹�˵ķ���Ϊ�ĸ�����
        for (i = 0; i < TANK_SHAPE_PNT_MAX; i++)
        {//�ֱ�����������̹�˵��������е�����һ������

            //�����̹�˻��������
            tank->dir = dr;
            tank->pnt.x = point->x - TANK_SHAPE_BOX[tank->dir][i].x;
            tank->pnt.y = point->y - TANK_SHAPE_BOX[tank->dir][i].y;

            if ((tank->pnt.x < 0) || (tank->pnt.x >= TankMaxX) ||
                (tank->pnt.y < 0) || (tank->pnt.y >= TankMaxY))
            {
                //���������ڽ���
                memset(tank, 0, sizeof(tank_t));
                tank->pr = PR_NULL;
                return RTN_NULL;
            }

            for (j = 0; j < TANK_SHAPE_PNT_MAX; j++)
            {//Ȼ�������������λ���Ƿ���ȷ
                pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][j].x;
                pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][j].y;
                if (tank_get_warmap(&pnt) != pr)
                {
                    break;
                }
            }

            if (j >= TANK_SHAPE_PNT_MAX)
            {
                //���ܻ����ĳ�����ϴ��ڶ���̹�ˣ������������ϵ�̹�˼���һ��
                //�������������
                return tank;//������һ��̹��
            }
        }
    }

    memset(tank, 0, sizeof(tank_t));
    tank->pr = PR_NULL;
    return RTN_NULL;
}

//�ڱ�����������̹��(��������)
tank_t* tank_search_atank_inbox(point_t* point)
{
    tank_t* tank = { 0 };
    point_t pnt = { 0 };
    //int32 pr = 0;
    int32 i = 0, j = 0;

    if (point == NULL)
    {
        return RTN_NULL;
    }

    if ((point->x < 0) || (point->x >= TankMaxX) ||
        (point->y < 0) || (point->y >= TankMaxY))
    {
        return RTN_NULL;
    }

    //pr = tank_get_warmap(point);

    //if ((pr != PR_ENMY) && (pr != PR_MYSELF))
    //if (pr == PR_NULL)
    //{
    //    return RTN_NULL;//����㲻����̹��
    //}

    //�����������̹�ˣ�����box������̹����Ϣ
    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        //if (TANK_WAR_BOX_VALID[i] == 0)//������Ч��Ԫ��
        if (TANK_WAR_BOX[i].pr == PR_NULL)//������Ч��Ԫ��
        {
            continue;
        }

        tank = &(TANK_WAR_BOX[i]);
        for (j = 0; j < TANK_SHAPE_PNT_MAX; j++)
        {
            pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][j].x;
            pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][j].y;
            if ((point->x == pnt.x) && (point->y == pnt.y))
            {
                return tank;
            }
    }
}

    return RTN_NULL;
}
#endif

//����һ���ڵ�
bomb_t* tank_create_abomb(tank_t* tank, bomb_t* bomb, int32* bnum)
{
    int32 i = 0;
    point_t pnt = { 0 };
    point_t* ppnt = NULL;
    if ((tank == NULL) || (bomb == NULL) || (bnum == NULL))
    {
        return RTN_NULL;
    }

    if (tank->pr == PR_NULL)
    {
        return RTN_NULL;
    }

    if (tank->wep > WP_NONE)//��ǿ��������ÿ�η��������ڵ�
    {
        //tank->wep -= 1;//����ʹ��һ��
        tank->bomb += 3;
        for (i = 0; i < 3; i++)
        {
            *bnum = i + 1;
            pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][i].x;
            pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][i].y;
            ppnt = tank_get_nextpnt(tank->dir, &pnt);
            if (ppnt == RTN_NULL)
            {
                *bnum = 0;
                memset(&(bomb[i]), 0, sizeof(bomb_t));
                bomb[i].valid = FALSE;
                bomb[i].pr = PR_NULL;
                return RTN_NULL;
            }

            bomb[i].valid = TRUE;
            bomb[i].dir = tank->dir;
            bomb[i].pr = tank->pr;
            bomb[i].ptank = tank;
            ppnt->col = TANK_PR_COLOUR[PR_BOMB];
            COPY_POINT(&(bomb[i].pnt), ppnt);
        }
    }
    else//��ͨ������ÿ�η���һ���ڵ�
    {
        tank->bomb += 1;
        *bnum = 1;
        pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][0].x;
        pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][0].y;
        ppnt = tank_get_nextpnt(tank->dir, &pnt);
        if (ppnt == RTN_NULL)
        {
            *bnum = 0;
            memset(&(bomb[0]), 0, sizeof(bomb_t));
            bomb[0].valid = FALSE;
            bomb[0].pr = PR_NULL;
            return RTN_NULL;
        }

        bomb[0].valid = TRUE;
        bomb[0].dir = tank->dir;
        bomb[0].pr = tank->pr;
        bomb[0].ptank = tank;
        ppnt->col = TANK_PR_COLOUR[PR_BOMB];
        COPY_POINT(&(bomb[0].pnt), ppnt);
    }

    return bomb;
}


//�ڵ�ͼ�ϻ����ڵ�
int32 tank_draw_abomb(bomb_t* bomb)
{
    int32 pr = 0;

    if (bomb == NULL)
    {
        return RTN_ERR;
    }

    if (bomb->valid == FALSE)
    {
        return RTN_ERR;
    }

    //�ڵ�Ҳ��Ϊ�з��ڵ����ҷ��ڵ�,��pr����
    //���ǲ��ܵз��ڵ������ҷ��ڵ�,��ʾ����״��
    //��һ����,����(��ɫ)Բ��,����Ҫע������
    //pr���ڿ�����ʾ����״
    pr = ((bomb->pr != PR_NULL) ? PR_BOMB : PR_NULL);
    tank_set_warmap(&(bomb->pnt), pr);

    return RTN_OK;
}

//����ڵ��ܷ�����ƶ�
int32 tank_check_abomb(bomb_t* bomb)
{
    //int32 i = 0;
    //point_t pnt = { 0 };
    //point_t* ppnt = NULL;

    if (bomb == NULL)
    {
        return FALSE;
    }

    if ((bomb->pr == PR_NULL) || (bomb->valid == FALSE))
    {
        return FALSE;
    }

    if ((bomb->pnt.x < 0) || (bomb->pnt.x >= TankMaxX) ||
        (bomb->pnt.y < 0) || (bomb->pnt.y >= TankMaxY))
    {
        return FALSE;
    }

#if 0
    pnt.x = bomb->pnt.x;
    pnt.y = bomb->pnt.y;

    ppnt = tank_get_nextpnt(bomb->dir, &pnt);

    if (ppnt == RTN_NULL)
    {
        return FALSE;
    }

    if ((ppnt->x < 0) || (ppnt->x >= TankMaxX) ||
        (ppnt->y < 0) || (ppnt->y >= TankMaxY))
    {
        return FALSE;
    }
#endif

    return TRUE;
}

//�ڵ�ҩ���в����ڵ�(��������)
bomb_t* tank_search_abomb_inbox(point_t* point)
{
    int32 i = 0;
    if (point == NULL)
    {
        return RTN_NULL;
    }

    if ((point->x < 0) || (point->x >= TankMaxX) ||
        (point->y < 0) || (point->y >= TankMaxY))
    {
        return RTN_NULL;
    }

    for (i = 0; i < TANK_BOMB_NUM_MAX; i++)
    {
        if ((TANK_BOMB_BOX[i].pr == PR_NULL) ||
            (TANK_BOMB_BOX[i].valid == FALSE))//������Ч��Ԫ��
        {
            continue;
        }

        if ((point->x == TANK_BOMB_BOX[i].pnt.x) &&
            (point->y == TANK_BOMB_BOX[i].pnt.y))
        {
            return &(TANK_BOMB_BOX[i]);
        }
    }

    return RTN_NULL;
}

//����һ��װ��
equip_t* tank_create_aequip(equip_t* equip)
{
    point_t pnt = { 0 };
    int32 n = TankMaxX * TankMaxY;
    int32 i = 0;

    if (equip == RTN_NULL)
    {
        return RTN_NULL;
    }

    equip->tmr = tank_get_rand(TANK_TIMER_EQUIP_MIN, TANK_TIMER_EQUIP_MAX);
    equip->pr = tank_get_rand(TANK_TIMER_MIN, TANK_TIMER_MAX);
    equip->valid = TRUE;
    if ((equip->pr % 2) == 0)
    {
        equip->pr = PR_LIFE;
    }
    else
    {
        equip->pr = PR_WEAPON;
    }

    while (n--)//Ѱ�ҿ��Է���װ���������
    {
        tank_get_randpnt(&(equip->pnt));//����һ�������

        pnt.x = equip->pnt.x;
        pnt.y = equip->pnt.y;
        if (tank_get_warmap(&pnt) == PR_NULL)
        {
            equip->pnt.col = TANK_PR_COLOUR[equip->pr];
            return equip;
        }
    }

    memset(equip, 0, sizeof(equip_t));
    equip->valid = FALSE;
    equip->pr = PR_NULL;

    return RTN_NULL;
}

//�ڵ�ͼ�ϻ���װ��
uint32 tank_draw_aequip(equip_t* equip)
{
    point_t pnt = { 0 };
    int32 n = TankMaxX * TankMaxY;
    int32 i = 0;

    if (equip == RTN_NULL)
    {
        return RTN_ERR;
    }

    if (equip->valid == FALSE)
    {
        return RTN_ERR;
    }

    tank_set_warmap(&(equip->pnt), equip->pr);

    return RTN_OK;
}

//̹���ƶ�
int32 tank_move_atank(dir_t dir)
{
    int32 i = 0;

    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        //if (TANK_WAR_BOX_VALID[i] == 0)//������Ч��Ԫ��
        if ((TANK_WAR_BOX[i].pr == PR_NULL) ||
            (TANK_WAR_BOX[i].valid == FALSE))
        {
            continue;
        }

        if (TANK_WAR_BOX[i].pr == PR_ENMY)//�о�
        {
            if (TANK_WAR_BOX[i].mv > MV_STOP)//������δ����
            {//�����ƶ�
                TANK_WAR_BOX[i].mv -= 1;
                if (tank_check_atank(&(TANK_WAR_BOX[i])) != FALSE)
                {//���ܼ����ƶ�
                    tank_get_nextpnt(TANK_WAR_BOX[i].dir, &(TANK_WAR_BOX[i].pnt));//̹���ƶ�һ��
                }
                else
                {//���ܼ����ƶ�����ת����
                    TANK_WAR_BOX[i].dir = tank_get_rand(DIR_MIN, DIR_MAX);//��ȡһ������ķ���
                }
            }
            else
            {//��ת����
                TANK_WAR_BOX[i].dir = tank_get_rand(DIR_MIN, DIR_MAX);//��ȡһ������ķ���
                TANK_WAR_BOX[i].mv = tank_get_rand(MV_MOVE, min(TankMaxX, TankMaxY));//����һ��������ƶ�����
            }
        }
        else if (TANK_WAR_BOX[i].pr == PR_MYSELF)//�Ҿ�
        {
            if (dir < DIR_MAX)
            {//�ƶ�
                if (TANK_WAR_BOX[i].dir == dir)
                {//�����ƶ�
                    if (tank_check_atank(&(TANK_WAR_BOX[i])) != FALSE)
                    {//���ܼ����ƶ�
                        tank_get_nextpnt(TANK_WAR_BOX[i].dir, &(TANK_WAR_BOX[i].pnt));//̹���ƶ�һ��
                    }
                }
                else
                {//��ת����
                    TANK_WAR_BOX[i].dir = dir;
                }
            }
        }
    }

    return RTN_OK;
}


//�ڵ��ƶ�
int32 tank_move_abomb(void)
{
    int32 i = 0;
    //point_t pnt = { 0 };
    //tank_t* ptank = NULL;

    for (i = 0; i < TANK_BOMB_NUM_MAX; i++)
    {
        //if (TANK_BOMB_BOX_VALID[i] == 0)//������Ч��Ԫ��
        if ((TANK_BOMB_BOX[i].pr == PR_NULL) ||
            (TANK_BOMB_BOX[i].valid == FALSE))
        {
            continue;
        }

        if (tank_check_abomb(&(TANK_BOMB_BOX[i])) == FALSE)
        {//���ܼ����ƶ�
            TANK_BOMB_BOX[i].pr = PR_NULL;
        }
        else
        {//�����ƶ�
            //COPY_POINT(&pnt, &(TANK_BOMB_BOX[i].pnt));
            //ptank = tank_search_atank_inbox(&pnt);
            //if (ptank != RTN_NULL)
            //{
                //tank_set_warmap(&pnt, ptank->pr);
                //tank_draw_atank(ptank);
            //}
            tank_get_nextpnt(TANK_BOMB_BOX[i].dir, &(TANK_BOMB_BOX[i].pnt));
        }
    }

    return RTN_OK;
}

//̹�˴�սװ��ϵͳ
int32 tank_move_equip(void)
{
    equip_t equip = { 0 };
    //����װ��
    if (TANK_PR.create_equp_tmr > 0)
    {
        //TANK_EQUIP.pr = PR_NULL;
        //TANK_EQUIP.valid = FALSE;
        TANK_PR.create_equp_tmr -= 1;
    }
    else
    {
        if (tank_create_aequip(&equip) != RTN_NULL)
        {
            memcpy(&TANK_EQUIP, &equip, sizeof(equip_t));//�����µ�
        }

        TANK_PR.create_equp_tmr = tank_get_rand(TANK_TIMER_EQUIP_MIN, TANK_TIMER_EQUIP_MAX);//�������ɶ�ʱ��
    }

    if ((TANK_EQUIP.pr != PR_NULL) && (TANK_EQUIP.valid != FALSE))
    {
        if (TANK_EQUIP.tmr > 0)
        {
            TANK_EQUIP.tmr -= 1;
        }
        else
        {
            TANK_EQUIP.pr = PR_NULL;
            TANK_EQUIP.valid = FALSE;
            TANK_PR.create_equp_tmr = tank_get_rand(TANK_TIMER_EQUIP_MIN, TANK_TIMER_EQUIP_MAX);//�������ɶ�ʱ��
        }
    }

    //DEBUG_LOG("TANK_PR.create_equp_tmr[%d], TANK_EQUIP.tmr[%d], TANK_EQUIP.pr[%d]",
    //    TANK_PR.create_equp_tmr, TANK_EQUIP.tmr, TANK_EQUIP.pr);

    return RTN_OK;
}

int32 tank_sound(int32 sid)
{
    if (sid)
    {//start
        PlaySound(MAKEINTRESOURCE(sid), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
    }
    else
    {//stop
        PlaySound(NULL, 0, 0);
    }
    return RTN_OK;
}

//̹�˳�ʼ��
int32 tank_init(void)
{
    int32 i = 0;
    int32 pr = 0;

    tank_clear_warmap();

    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        memset(&(TANK_WAR_BOX[i]), 0, sizeof(tank_t));
        TANK_WAR_BOX[i].pr = PR_NULL;
        //TANK_WAR_BOX_VALID[i] = FALSE;
        TANK_WAR_BOX[i].valid = FALSE;
    }

    for (i = 0; i < TANK_BOMB_NUM_MAX; i++)
    {
        memset(&(TANK_BOMB_BOX[i]), 0, sizeof(bomb_t));
        TANK_BOMB_BOX[i].pr = PR_NULL;
        //TANK_BOMB_BOX_VALID[i] = FALSE;
        TANK_BOMB_BOX[i].valid = FALSE;
    }

    memset(&(TANK_PR), 0, sizeof(tank_pr_t));
    memset(&(TANK_EQUIP), 0, sizeof(equip_t));
    TANK_EQUIP.pr = PR_NULL;
    TANK_EQUIP.valid = FALSE;

    for (i = 0; i < TANK_ENMY_NUM_INIT; i++)
    {
        if (i == 0)
        {
            pr = PR_MYSELF;
        }
        else
        {
            pr = PR_ENMY;
        }

        if (tank_create_atank(&(TANK_WAR_BOX[i]), pr) == RTN_NULL)
        {
            return RTN_ERR;
        }

        TANK_PR.cur_tank_num++;
    }

    TANK_PR.war_tank_num = TANK_PR.cur_tank_num;
    //TANK_PR.cur_bomb_num = 0;
    TANK_PR.create_equp_tmr = tank_get_rand(TANK_TIMER_EQUIP_MIN, TANK_TIMER_EQUIP_MAX);
    TANK_PR.speed = TANK_SPEED_MAX;
    TANK_PR.debug = FALSE;
    TANK_PR.super = FALSE;

    tank_update_warmap();

    tank_sound(IDR_WAVE_START);

    return RTN_OK;
}

//̹���ƶ����(��ײ���)
int32 tank_detect(tank_t* tank)
{
    int32 i = 0, pr = 0;
    point_t pnt = { 0 };
    bomb_t* bomb = NULL;

    if (tank == NULL)
    {
        return RTN_ERR;
    }

    if ((tank->pr == PR_NULL) || (tank->valid == FALSE))
    {
        return RTN_OK;
    }

    for (i = 0; i < TANK_SHAPE_PNT_MAX; i++)
    {
        pnt.x = tank->pnt.x + TANK_SHAPE_BOX[tank->dir][i].x;
        pnt.y = tank->pnt.y + TANK_SHAPE_BOX[tank->dir][i].y;

        pr = tank_get_warmap(&(pnt));
        switch (pr)
        {
        case PR_WEAPON://�Ե�����
            tank->wep += TANK_WEAPON_LIFE_MAX;//������������ʹ�ô���
            TANK_EQUIP.tmr = 0;//װ����ʧ
            TANK_EQUIP.pr = PR_NULL;
            tank_sound(IDR_WAVE_WEAPON);
            break;
        case PR_LIFE://�Ե�װ��
            tank->lf += 1;//�Ե�װ��������ֵ+1
            TANK_EQUIP.tmr = 0;//װ����ʧ
            TANK_EQUIP.pr = PR_NULL;
            tank_sound(IDR_WAVE_LIFE);
            break;
        case PR_BOMB://�Ե��ڵ�
            bomb = tank_search_abomb_inbox(&(pnt));
            if (bomb == RTN_NULL)
            {
                break;
            }
            if (bomb->pr != tank->pr)//�Լ����ڵ�����ը�Լ�
            {
                if (tank->lf > LF_LIVE)
                {
                    tank->lf -= 1;//����ֵ-1
                }
                if (tank->lf = LF_LIVE)
                {
                    tank->lf -= 1;//����ֵ-1
                    bomb->ptank->kill += 1;//Ϊ����ڵ�����������һ�λ�ɱ��¼
                }
                bomb->pr = PR_NULL;//����Ŀ����ڵ�ʧЧ
                tank->wep = WP_NONE;//̹�˱�����֮��������ʧЧ
                tank_sound(IDR_WAVE_BOMB);
            }
            break;
        case PR_NULL:
        case PR_WALL:
        case PR_ENMY:
        case PR_MYSELF:
        default:
            break;
        }
    }

    return RTN_OK;
}

int32 tank_count(void)
{
    int32 i = 0;
    TANK_PR.cur_tank_num = 0;
    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        if ((TANK_WAR_BOX[i].pr != PR_NULL) &&
            (TANK_WAR_BOX[i].valid != FALSE))
        {
            TANK_PR.cur_tank_num += 1;//ͳ��̹�˵���Ŀ
        }
    }

    return RTN_OK;
}

#if 0
int32 tank_cbomb(void)
{
    TANK_PR.cur_bomb_num = 0;//ͳ���ڵ���Ŀ
    for (i = 0; i < TANK_BOMB_NUM_MAX; i++)
    {
        if ((TANK_BOMB_BOX[i].pr != PR_NULL) &&
            (TANK_BOMB_BOX[i].valid != FALSE))
        {
            TANK_PR.cur_bomb_num += 1;//ͳ���ڵ���Ŀ
        }
    }
}
#endif

//����ս��
int32 tank_clean(tank_t* tank)
{
    int32 i = 0, j = 0, k = 0;

    static int8 war_num_flag = FALSE;

    tank_t ttank = { 0 };
    tank_t* ptank = NULL;

    if (tank == NULL)
    {
        return RTN_ERR;
    }

    if ((tank->pr == PR_NULL) || (tank->valid == FALSE))
    {
        return RTN_OK;
    }

    //��ɨս��
    if (tank->lf == LF_DIE)
    {//������Ѿ�ը�ٵ�̹��

        if (tank->pr == PR_MYSELF)
        {//�Ҿ�ս��
            tank->pr = PR_NULL;
            //DEBUG_LOG("");
            return RTN_ERR;
        }

        if (tank->pr == PR_ENMY)
        {//�о����һ��̹��
           //����һ����ٵ�̹��
            tank->pr = PR_NULL;
#if 0
            //��Ԯһ����̹��
            ptank = tank_create_atank(&ttank, PR_ENMY);
            if (ptank != RTN_NULL)
            {
                //Ѱ��λ�ã�����̹�˲��뵽������
                for (j = 0; j < TANK_WAR_NUM_MAX; j++)
                {
#if 0
                    if ((ptank->pnt.x == tank->pnt.x) &&
                        (ptank->pnt.y == tank->pnt.y))
                    {//��λ�ò����ڸ���ٵ��Ǹ�λ����
                        continue;
                    }

                    if ((TANK_WAR_BOX[j].pr == PR_NULL) ||
                        (TANK_WAR_BOX[j].valid == FALSE))
#else
                    if (TANK_WAR_BOX[j].valid == FALSE)
#endif
                    {
                        memcpy(&(TANK_WAR_BOX[j]), ptank, sizeof(tank_t));
                        break;
                    }
    }
}
#endif
}
    }

    if (tank->lf == LF_BURN)
    {//������ȼ�յ�̹�˱��Ϊը�٣��´�����
        tank->lf = LF_DIE;
        tank->mv = MV_STOP;//ֹͣ�ƶ�
        //tank->pr = PR_BOMB;//�������Ը�Ϊ�ڵ�����ʾ������ը������̹��ײ����Ҳ��ʧȥһ������ֵ
        tank->pnt.col = TANK_PR_COLOUR[PR_BOMB];//������ɫ��Ϊ���ڵ�ͬɫ
    }

#if 1
#if 1
    //�Ҿ�ÿ��ɱ5��̹�˵о�̹������+1
    if ((tank->pr == PR_MYSELF) && (tank->kill > 0))
    {
        if ((tank->kill % 5) == 0)
        {
            if (war_num_flag == FALSE)
            {
                war_num_flag = TRUE;
                TANK_PR.war_tank_num += 1;
            }
        }
        else
        {
            war_num_flag = FALSE;
        }
    }
#endif

    tank_count();//ͳ��̹�˵���Ŀ

    //��Ԯ��̹��
    k = 0;
    for (i = TANK_PR.cur_tank_num; i < TANK_PR.war_tank_num; i++)
    {
        ptank = tank_create_atank(&ttank, PR_ENMY);
        if (ptank != RTN_NULL)
        {
            //Ѱ��λ�ã�����̹�˲��뵽������
            for (j = k; j < TANK_WAR_NUM_MAX; j++)
            {
                if ((TANK_WAR_BOX[j].pr == PR_NULL) &&
                    (TANK_WAR_BOX[j].valid == FALSE))
                {
                    k = j + 1;
                    memcpy(&(TANK_WAR_BOX[j]), ptank, sizeof(tank_t));
                    break;
                }
            }
        }
    }

#endif
    return RTN_OK;
}

//̹�˿���
int32 tank_fire(tank_t* tank, int32* fire)
{
    int32 m = 0, k = 0, j = 0;

    int32 bnum = 0;
    bomb_t bomb[3] = { 0 };

    if ((tank == NULL) || (fire == NULL))
    {
        return RTN_ERR;
    }

    if ((tank->pr == PR_NULL) || (tank->valid == FALSE))
    {
        return RTN_OK;
    }

    if (tank->pr == PR_MYSELF)
    {
        //�Ҿ�����ʽ���ֶ�����
        if (*fire)
        {
            *fire = FALSE;
            tank->fire = 0;
        }
        else
        {
            tank->fire = 1;
        }
    }

    //���𵹼�ʱ
    if (tank->fire > 0)
    {
        tank->fire -= 1;
    }
    else
    {
        tank->fire = tank_get_rand(TANK_TIMER_FIRE_MIN, TANK_TIMER_FIRE_MAX);
        //̹�˷����ڵ�(�Ȳ����ڵ������������)
        if (tank_create_abomb(tank, bomb, &bnum) != RTN_NULL)
        {
            m = 0;
            //���ڵ����뵽�ڵ�������
            for (j = 0; j < bnum; j++)
            {
                //Ѱ�ҿ��Բ����ڵ���λ��
                for (k = m; k < TANK_BOMB_NUM_MAX; k++)
                {
                    if ((TANK_BOMB_BOX[k].pr == PR_NULL) ||
                        (TANK_BOMB_BOX[k].valid == FALSE))
                    {
                        //�����ڵ�
                        m = k + 1;
                        memcpy(&(TANK_BOMB_BOX[k]), &bomb[j], sizeof(bomb_t));
                        break;
                    }
                }
            }

            if (tank->pr == PR_MYSELF)
            {
                tank_sound(IDR_WAVE_FIRE);
            }
        }
    }

    return RTN_OK;
}

void tank_draw(void)
{
    int32 i = 0;

    tank_clear_warmap();

    //����̹��
    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        if (TANK_WAR_BOX[i].valid == FALSE)
        {
            continue;
        }

        tank_draw_atank(&(TANK_WAR_BOX[i]));
        if (TANK_WAR_BOX[i].pr == PR_NULL)
        {
            memset(&(TANK_WAR_BOX[i]), 0, sizeof(tank_t));
            TANK_WAR_BOX[i].pr = PR_NULL;
            TANK_WAR_BOX[i].valid = FALSE;
        }
    }

    //����װ��
    if (TANK_EQUIP.valid != FALSE)
    {
        tank_draw_aequip(&(TANK_EQUIP));
        if (TANK_EQUIP.pr == PR_NULL)
        {
            memset(&(TANK_EQUIP), 0, sizeof(equip_t));
            TANK_EQUIP.pr = PR_NULL;
            TANK_EQUIP.valid = FALSE;
        }
    }


    //�����ڵ�
    for (i = 0; i < TANK_BOMB_NUM_MAX; i++)
    {
        if (TANK_BOMB_BOX[i].valid == FALSE)
        {
            continue;
        }
        tank_draw_abomb(&(TANK_BOMB_BOX[i]));
        if (TANK_BOMB_BOX[i].pr == PR_NULL)
        {
            memset(&(TANK_BOMB_BOX[i]), 0, sizeof(bomb_t));
            TANK_BOMB_BOX[i].pr = PR_NULL;
            TANK_BOMB_BOX[i].valid = FALSE;
        }
    }

    tank_update_warmap();
}

int32 tank_run(dir_t* dir, int32* fire, int32 super, int32 debug)
{
    int32 i = 0, j = 0, k = 0, m = 0;
    equip_t equip = { 0 };
    tank_t tank = { 0 };
    tank_t* ptank = NULL;
    bomb_t bomb[3] = { 0 };
    int32 bnum = 0;
    static int32 speed = 0;
    int32 pr = 0;

    int32 ret = RTN_OK;

    TANK_PR.debug = (debug ? TRUE : FALSE);
    TANK_PR.super = (super ? TRUE : FALSE);

#if 0
    if (tnum != 0)
    {
        tnum = 0;
        tnum = (tnum <= TANK_ALL_NUM_MIN) ? TANK_ALL_NUM_MIN :
            ((tnum_t >= TANK_ALL_NUM_MIN) ? TANK_ALL_NUM_MIN : tnum);

        TANK_PR.war_tank_num = tnum;
    }
#endif

    for (i = 0; i < TANK_WAR_NUM_MAX; i++)
    {
        //�޵�ģʽ
        if (TANK_WAR_BOX[i].pr == PR_MYSELF)
        {
            tank_set_super(&(TANK_WAR_BOX[i]), TANK_PR.super);
        }

        //��ɨս��
        ret = tank_clean(&(TANK_WAR_BOX[i]));
        if (ret != RTN_OK)
        {
            //DEBUG_LOG("ERR");
            return ret;
        }

        //̹�˿���
        tank_fire(&(TANK_WAR_BOX[i]), fire);

        //��ײ���
        tank_detect(&(TANK_WAR_BOX[i]));

    }

    //�ƶ�̹��,�ڵ���װ��
    if (speed < TANK_PR.speed)
    {
        speed++;//�ڵ��ٶȱ�̹�˿�,�ڵ��ƶ�5��̹���ƶ�һ��
    }
    else
    {
        speed = 0;
        tank_move_atank(*dir);
        *dir = DIR_MAX;
    }

    tank_move_equip();
    tank_move_abomb();

    tank_draw();

    return RTN_OK;
    }
