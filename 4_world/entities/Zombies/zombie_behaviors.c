class NY_ZombieSoldier8 : NY_ZombieSoldier_Base 
{
    protected ref Timer m_ToxicGasTimer;
    protected vector m_DeathPosition;
    protected const float TOXIC_GAS_RADIUS = 20.0;
    protected const float TOXIC_GAS_DURATION = 300.0;
    protected ParticleSource m_ParticleExploded;
    protected EffectSound m_ExplosionSound;
    protected bool m_Exploded;
    
    void NY_ZombieSoldier8()
    {
    }

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);
        
        m_DeathPosition = GetPosition();
        
        if (GetGame().IsServer())
        {
            // 创建污染区域
            GetGame().CreateObject("ContaminatedArea_Local", m_DeathPosition);
            
            // 启动伤害计时器
            m_ToxicGasTimer = new Timer();
            m_ToxicGasTimer.Run(1.0, this, "CheckPlayersInToxicArea", null, TOXIC_GAS_DURATION);
            
            m_Exploded = true;
        }
        
        // 在客户端创建效果
        if (!GetGame().IsDedicatedServer())
        {
            // 播放爆炸音效和粒子
            ClearFlags(EntityFlags.VISIBLE, false);
            m_ParticleExploded = ParticleManager.GetInstance().PlayInWorld(ParticleList.GRENADE_CHEM_BREAK, m_DeathPosition);
            PlaySoundSet(m_ExplosionSound, "Grenade_detonation_SoundSet", 0, 0);
            
            // 在僵尸身上创建毒气效果
            SetContaminatedEffect(true, -1, 
                ParticleList.CONTAMINATED_AREA_GAS_AROUND,
                ParticleList.CONTAMINATED_AREA_GAS_TINY,
                true, 1);
        }
    }

    override void OnExplosionEffects(Object source, Object directHit, int componentIndex, string surface, vector pos, vector surfNormal, float energyFactor, float explosionFactor, bool isWater, string ammoType)
    {
        if (!GetGame().IsDedicatedServer())
        {
            // 计算爆炸方向
            vector n = surfNormal.VectorToAngles() + "0 90 0";
            
            // 创建毒气爆炸效果
            Particle p1 = ParticleManager.GetInstance().PlayInWorld(ParticleList.GRENADE_CHEM_BREAK, pos);
            p1.SetOrientation(n);
            
            // 创建持续的毒气效果
            Particle p2 = ParticleManager.GetInstance().PlayInWorld(ParticleList.CONTAMINATED_AREA_GAS_AROUND, pos);
            p2.SetWiggle(7, 0.3);
            
            // 创建地面毒气效果
            Particle p3 = ParticleManager.GetInstance().PlayInWorld(ParticleList.CONTAMINATED_AREA_GAS_GROUND, pos);
            
            // 创建大范围毒气效果
            Particle p4 = ParticleManager.GetInstance().PlayInWorld(ParticleList.CONTAMINATED_AREA_GAS_BIGASS, pos);
        }
    }

    void CheckPlayersInToxicArea()
    {
        if (!GetGame().IsServer())
            return;

        array<Object> objects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        GetGame().GetObjectsAtPosition(m_DeathPosition, TOXIC_GAS_RADIUS, objects, proxyCargos);
        
        foreach (Object obj : objects)
        {
            PlayerBase player = PlayerBase.Cast(obj);
            if (player)
            {
                // 对玩家造成伤害
                player.SetHealth("", "Health", player.GetHealth("", "Health") - 5);
                player.GetStaminaHandler().DepleteStamina(EStaminaModifiers.HOLD_BREATH);
                
                // 给玩家添加毒气效果
                player.SetContaminatedEffect(true, -1, 
                    ParticleList.CONTAMINATED_AREA_GAS_AROUND,
                    ParticleList.CONTAMINATED_AREA_GAS_TINY,
                    true, 1);
            }
        }
    }

    override void EEDelete(EntityAI parent)
    {
        super.EEDelete(parent);
        
        if (m_ToxicGasTimer)
        {
            m_ToxicGasTimer.Stop();
        }
        
        if (m_ParticleExploded)
        {
            m_ParticleExploded.Stop();
        }
        
        // 移除毒气效果
        SetContaminatedEffect(false);
    }
}

class NY_ZombieSoldier7 : NY_ZombieSoldier_Base 
{
    // ... 其他属性保持不变 ...
    protected const float MOVEMENT_SPEED_REDUCTION = 0.7; // 修改为 0.7，让玩家可以移动，但速度减慢

    void SlowPlayerMovement(PlayerBase player)
    {
        if (!player)
            return;

        // 轻微消耗体力
        player.GetStaminaHandler().DepleteStamina(EStaminaModifiers.HOLD_BREATH);
        
        // 只使用疲劳效果，移除溺水效果
        player.GetModifiersManager().ActivateModifier(eModifiers.MDF_FATIGUE);
        
        // 获取玩家的移动控制器并减缓移动速度
        HumanMovementState state = new HumanMovementState();
        player.GetMovementState(state);
        
        if (state)
        {
            DayZPlayerImplement playerImpl = DayZPlayerImplement.Cast(player);
            if (playerImpl)
            {
                // 使用较温和的速度减缓
                HumanInputController input = playerImpl.GetInputController();
                if (input)
                {
                    input.OverrideMovementSpeed(true, MOVEMENT_SPEED_REDUCTION);
                }
            }
        }
    }

    // ... 其他方法保持不变 ...
} 