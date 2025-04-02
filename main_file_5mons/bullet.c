#include "Bullet.h"
#include "Monster.h" // 需你自己实现：至少要能获取怪物位置、让怪物掉血等
#include <stdlib.h>
#include <math.h>

/*
 * 创建并初始化子弹
 */
Bullet *bullet_create(struct Monster *target, float start_x, float start_y,
                      float speed, int damage)
{
    Bullet *bullet = (Bullet *)malloc(sizeof(Bullet));
    if (!bullet)
    {
        return NULL;
    }
    bullet->target = target;
    bullet->speed = speed;
    bullet->damage = damage;
    bullet->x = start_x;
    bullet->y = start_y;
    return bullet;
}

/*
 * 销毁子弹
 */
void bullet_destroy(Bullet *bullet)
{
    if (!bullet)
        return;
    free(bullet);
}

/*
 * 子弹向目标移动
 * 每次调用时，按照 speed 计算新的位置。
 * 若已到达目标，则调用 bullet_onHitMonster，并销毁子弹。
 */
void bullet_moveToTarget(Bullet *bullet)
{
    if (!bullet)
        return;

    // 如果没有目标，直接销毁
    if (!bullet->target)
    {
        bullet_destroy(bullet);
        return;
    }

    // 获取目标怪物位置
    float tx = monster_getPositionX(bullet->target);
    float ty = monster_getPositionY(bullet->target);

    // 计算子弹与目标的距离
    float dx = tx - bullet->x;
    float dy = ty - bullet->y;
    float dist = sqrtf(dx * dx + dy * dy);

    // 如果目标位置和子弹当前位置重合度很高，或者非常接近，则命中
    if (dist < 1.0f)
    {
        bullet_onHitMonster(bullet, bullet->target);
        bullet_destroy(bullet);
        return;
    }

    // 否则，根据 speed 让子弹朝着目标方向移动
    // 注：如果你的 speed 是“像素/帧”，则每次调用都会移动 speed 像素距离
    //    如果 dist < speed，则意味本次就能到达目标。
    //    具体怎么处理由你决定，这里做简单处理：
    if (dist <= bullet->speed)
    {
        // 本帧就能到达目标
        bullet->x = tx;
        bullet->y = ty;
        bullet_onHitMonster(bullet, bullet->target);
        bullet_destroy(bullet);
    }
    else
    {
        // 按照归一化方向移动
        float nx = dx / dist; // 方向单位向量 X
        float ny = dy / dist; // 方向单位向量 Y
        bullet->x += nx * bullet->speed;
        bullet->y += ny * bullet->speed;
    }
}

/*
 * 设置子弹目标
 */
void bullet_setTarget(Bullet *bullet, struct Monster *target)
{
    if (!bullet)
        return;
    bullet->target = target;
}

/*
 * 命中怪物时，执行伤害逻辑
 */
void bullet_onHitMonster(Bullet *bullet, struct Monster *monster)
{
    if (!bullet || !monster)
        return;

    // 对怪物造成伤害
    monster_getAttacked(monster, bullet->damage);
}
