#ifndef __BULLET_H__
#define __BULLET_H__

#ifdef __cplusplus
extern "C"
{
#endif

    // 先声明 Monster 结构体，具体定义可在 Monster.h 里
    struct Monster;

    /*
     * Bullet 结构体：用来存储子弹相关信息
     */
    typedef struct Bullet
    {
        struct Monster *target; // 目标怪物
        float speed;            // 子弹飞行速度（单位/帧 或 单位/秒，由你决定）
        int damage;             // 子弹伤害值

        float x; // 子弹当前位置 X
        float y; // 子弹当前位置 Y

        // 你还可以扩展更多字段，如子弹图片ID、类型等
    } Bullet;

    /*
     * 创建并初始化 Bullet
     * 参数：
     *   target: 目标怪物指针
     *   start_x, start_y: 子弹的起始坐标
     *   speed: 子弹移动速度
     *   damage: 子弹伤害
     * 返回：新分配的子弹指针
     */
    Bullet *bullet_create(struct Monster *target, float start_x, float start_y,
                          float speed, int damage);

    /*
     * 销毁 Bullet，并释放相关资源
     */
    void bullet_destroy(Bullet *bullet);

    /*
     * 子弹向目标移动。该函数可在游戏循环里被频繁调用，让子弹逐帧（或周期）更新位置。
     * 如果到达目标，则对子弹目标造成伤害，并自动销毁子弹。
     */
    void bullet_moveToTarget(Bullet *bullet);

    /*
     * 设置子弹目标
     */
    void bullet_setTarget(Bullet *bullet, struct Monster *target);

    /*
     * 当子弹命中怪物时，执行伤害逻辑
     */
    void bullet_onHitMonster(Bullet *bullet, struct Monster *monster);

#ifdef __cplusplus
}
#endif

#endif // __BULLET_H__
